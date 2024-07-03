/*=============================================================================
# Filename: StringIndex.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-09-15
# Description: implement functions in StringIndex.h
=============================================================================*/

#include "StringIndex.h"
#include <pthread.h>
#include <unistd.h>
using namespace std;

void StringIndexFile::setNum(unsigned _num)
{
	this->num = _num;
}
unsigned StringIndexFile::getNum()
{
        return this->num;
}

void StringIndexFile::SetTrie(Trie *trie0)
{
	this->trie = trie0;
}

void StringIndexFile::save(KVstore &kv_store)
{
	this->index_file = fopen((this->loc + "index").c_str(), "wb");
	if (this->index_file == NULL)
	{
		cerr << "save " << this->loc + "index" << " for wb error." << endl;
		return;
	}
	this->value_file = fopen((this->loc + "value").c_str(), "wb");
	if (this->value_file == NULL)
	{
		cerr << "save " << this->loc + "value" << " for wb error." << endl;
		return;
	}

	fwrite(&this->num, sizeof(unsigned), 1, this->index_file);

	long offset = 0;
	for (unsigned i = 0; i < this->num; i++)
	{
		string str;
		if (this->type == Entity)
			str = kv_store.getEntityByID(i,false);
		if (this->type == Literal)
			str = kv_store.getLiteralByID(Util::LITERAL_FIRST_ID + i,false);
		if (this->type == Predicate)
			str = kv_store.getPredicateByID(i,false);
		unsigned length = str.length();
		fwrite(&offset, sizeof(long), 1, this->index_file);
		fwrite(&length, sizeof(unsigned), 1, this->index_file);
		offset += length;
		fwrite(str.c_str(), sizeof(char), length, this->value_file);
	}

	fclose(this->index_file);
	fclose(this->value_file);
	this->index_file = NULL;
	this->value_file = NULL;
}

void StringIndexFile::load()
{
	this->index_file = fopen((this->loc + "index").c_str(), "rb+");
	if (this->index_file == NULL)
	{
		cerr << "load " << this->loc + "index" << " for rb+ error." << endl;
		return;
	}
	this->value_file = fopen((this->loc + "value").c_str(), "rb+");
	if (this->value_file == NULL)
	{
		cerr << "load " << this->loc + "value" << " for rb+ error." << endl;
		return;
	}

	fread(&this->num, sizeof(unsigned), 1, this->index_file);

	(*this->index_table).resize(this->num);
	for (unsigned i = 0; i < this->num; i++)
	{
		fread(&(*this->index_table)[i].offset, sizeof(long), 1, this->index_file);
		fread(&(*this->index_table)[i].length, sizeof(unsigned), 1, this->index_file);
		this->empty_offset = max(this->empty_offset, (*this->index_table)[i].offset + (long)(*this->index_table)[i].length);
	}
	//	trie->LoadTrie(dictionary_path);
}

bool StringIndexFile::randomAccess(unsigned id, string *str, char* &buffer, unsigned &buffer_size, bool real)
{
	if (id >= this->num)
		return false;

	long offset = (*this->index_table)[id].offset;
	unsigned length = (*this->index_table)[id].length;
	//if(id == 9)
	//{
	//cout<<"check: "<<offset<<" "<<length<<endl;
	//}

	allocBuffer(buffer, buffer_size, length);

	//DEBUG: here a bug exists if we use pread instead of fread, the details are in BUG_StringIndex_pread of docs/BUGS.md
	//fseek(this->value_file, offset, SEEK_SET);
	// fread(this->buffer, sizeof(char), length, this->value_file);
	pread(fileno(value_file), buffer, sizeof(char)*length, offset);
	buffer[length] = '\0';
	char* UncompressBuffer = new char[MAX_BLOCK_SIZE*4];
	if (real)
		trie->Uncompress(buffer, length, *str, UncompressBuffer);
	else
		str->assign(buffer, length);
	delete[] UncompressBuffer;
	//*str = string(this->buffer);

	//if(id == 9)
	//{
	//cout<<"check: "<<*str<<endl;
	//}

	//	if (real)
	//	{
	//		*str = trie->Uncompress(*str, str->length());//Uncompresss
	//	}
	//if(id == 9)
	//{
	//cout<<"check: "<<*str<<endl;
	//}

	return true;
}

void StringIndexFile::trySequenceAccess(std::vector<StringIndexFile::AccessRequest> request, char* &buffer, unsigned &buffer_size, bool real)
{
	if (request.empty())
		return;

	long min_begin = -1, max_end = 0;
	for (int i = 0; i < (int)request.size(); i++)
	{
		if (min_begin == -1)
			min_begin = request[i].offset;
		else
			min_begin = min(min_begin, request[i].offset);

		max_end = max(max_end, request[i].offset + long(request[i].length));
	}

	std::string code_print;
	if (this->type == Entity)
		code_print = "Entity StringIndex ";
	if (this->type == Literal)
		code_print = "Literal StringIndex ";
	if (this->type == Predicate)
		code_print = "Predicate StringIndex ";

	long current_offset = 0;
	if ((max_end - min_begin) / 800000L < (long)request.size())
	{
		SLOG_CODE(code_print << "sequence access.");

#ifndef PARALLEL_SORT
		sort(request.begin(), request.end());
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(this->request.begin(), this->request.end());
#endif
		int pos = 0;
		char *block = new char[MAX_BLOCK_SIZE];

		long current_block_begin = min_begin;

		while (current_block_begin < max_end)
		{
			long current_block_end = min(current_block_begin + MAX_BLOCK_SIZE, max_end);

			if (current_block_end <= request[pos].offset)
			{
				current_block_begin = request[pos].offset;
				current_block_end = min(current_block_begin + MAX_BLOCK_SIZE, max_end);
			}

			// fread(block, sizeof(char), current_block_end - current_block_begin, this->value_file);
			pread(fileno(this->value_file), block, sizeof(char)*(current_block_end-current_block_begin), current_block_begin);

			while (pos < (int)request.size())
			{
				long offset = request[pos].offset;
				long length = request[pos].length;

				if (offset >= current_block_end)
					break;

				if (current_block_begin <= offset && offset + length <= current_block_end)
				{
					allocBuffer(buffer, buffer_size, length);
					memcpy(buffer, &block[offset - current_block_begin], length);
					buffer[length] = '\0';
					*request[pos].str = string(buffer);

//					*this->request[pos].str = trie->Uncompress(
//					*this->request[pos].str, this->request[pos].str->length());

					pos++;
				}
				else if (current_block_begin <= offset)
				{
					length = current_block_end - offset;
					allocBuffer(buffer, buffer_size, length);
					memcpy(buffer, &block[offset - current_block_begin], length);
					buffer[length] = '\0';
					*request[pos].str = string(buffer);
					break;
				}
				else if (offset + length <= current_block_end)
				{
					length = offset + length - current_block_begin;
					allocBuffer(buffer, buffer_size, length);
					memcpy(buffer, block, length);
					buffer[length] = '\0';
					*request[pos].str += string(buffer);

//					*this->request[pos].str = trie->Uncompress(
//					*this->request[pos].str, this->request[pos].str->length());

					pos++;
					while (pos < (int)request.size() && request[pos - 1].offset == request[pos].offset)
					{
						*request[pos].str = *request[pos - 1].str;
						pos++;
					}
				}
				else
				{
					length = current_block_end - current_block_begin;
					allocBuffer(buffer, buffer_size, length);
					memcpy(buffer, block, length);
					buffer[length] = '\0';
					*request[pos].str += string(buffer);
					break;
				}
			}

			current_block_begin = current_block_end;
		}
		delete[] block;
	}
	else
	{
		SLOG_CODE(code_print << "random access.");

		for (int i = 0; i < (int)request.size(); i++)
			this->randomAccess(request[i].id, request[i].str, buffer, buffer_size, real);
	}
	request.clear();
}


void StringIndexFile::change(unsigned id, KVstore &kv_store)
{
	//DEBUG: for predicate, -1 when invalid
	if (id == INVALID)	return;

	if (this->num <= id)
	{
		while (this->num <= id)
		{
			(*this->index_table).push_back(IndexInfo());

			fseek(this->index_file, sizeof(unsigned) + this->num * (sizeof(long) + sizeof(unsigned)), SEEK_SET);
			fwrite(&(*this->index_table)[this->num].offset, sizeof(long), 1, this->index_file);
			fwrite(&(*this->index_table)[this->num].length, sizeof(unsigned), 1, this->index_file);

			this->num++;
		}

		fseek(this->index_file, 0, SEEK_SET);
		fwrite(&this->num, sizeof(unsigned), 1, this->index_file);
	}

	string str;
	if (this->type == Entity)
		str = kv_store.getEntityByID(id,false);
	if (this->type == Literal)
		str = kv_store.getLiteralByID(Util::LITERAL_FIRST_ID + id, false);
	if (this->type == Predicate)
		str = kv_store.getPredicateByID(id, false);

	(*this->index_table)[id].offset = this->empty_offset;
	(*this->index_table)[id].length = str.length();
	this->empty_offset += (*this->index_table)[id].length;

	fseek(this->index_file, sizeof(unsigned) + id * (sizeof(long) + sizeof(unsigned)), SEEK_SET);
	fwrite(&(*this->index_table)[id].offset, sizeof(long), 1, this->index_file);
	fwrite(&(*this->index_table)[id].length, sizeof(unsigned), 1, this->index_file);

	fseek(this->value_file, (*this->index_table)[id].offset, SEEK_SET);
	fwrite(str.c_str(), sizeof(char), (*this->index_table)[id].length, this->value_file);
    fflush(this->index_file);
	fflush(this->value_file);
	//if(id == 9)
	//{
		//cout<<"check in change():9 "<<str<<endl;
		//string str2;
		//randomAccess(id, &str2);
		//cout<<str2<<endl;
	//}
}

void StringIndexFile::disable(unsigned id)
{
	//DEBUG: for predicate, -1 when invalid
	if (id >= this->num)	return;

	(*this->index_table)[id] = IndexInfo();

	fseek(this->index_file, sizeof(unsigned) + id * (sizeof(long) + sizeof(unsigned)), SEEK_SET);
	fwrite(&(*this->index_table)[id].offset, sizeof(long), 1, this->index_file);
	fwrite(&(*this->index_table)[id].length, sizeof(unsigned), 1, this->index_file);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void StringIndex::setNum(StringIndexFile::StringIndexFileType _type, unsigned _num)
{
	if (_type == StringIndexFile::Entity)
		this->entity.setNum(_num);
	if (_type == StringIndexFile::Literal)
		this->literal.setNum(_num);
	if (_type == StringIndexFile::Predicate)
		this->predicate.setNum(_num);
}

unsigned StringIndex::getNum(StringIndexFile::StringIndexFileType _type)
{
        if (_type == StringIndexFile::Entity)
                return this->entity.getNum();
        if (_type == StringIndexFile::Literal)
                return this->literal.getNum();
        if (_type == StringIndexFile::Predicate)
                return this->predicate.getNum();
		assert(false);
}

void StringIndex::save(KVstore &kv_store)
{
	this->entity.save(kv_store);
	this->literal.save(kv_store);
	this->predicate.save(kv_store);
}

void StringIndex::load()
{
	this->entity.load();
	this->literal.load();
	this->predicate.load();
}

bool
StringIndex::searchBuffer(unsigned _id, string* _str)
{
	if(_id < Util::LITERAL_FIRST_ID) //entity
	{
		if(_id < this->entity_buffer_size)
		{
			*_str = this->entity_buffer->get(_id);
			return true;
		}
		return false;
	}
	else //literal
	{
		_id -= Util::LITERAL_FIRST_ID;
		if(_id < this->literal_buffer_size)
		{
			*_str = this->literal_buffer->get(_id);
			return true;
		}
		return false;
	}
}

bool StringIndex::randomAccess(unsigned id, string *str, char* &buffer, unsigned &buffer_size, bool is_entity_or_literal, bool real)
{
	if(id < 0) return false;

	if (is_entity_or_literal)
	{
		if (id < Util::LITERAL_FIRST_ID)
		{
			return this->entity.randomAccess(id, str, buffer, buffer_size, real);
		}
		else
		{
			return this->literal.randomAccess(id - Util::LITERAL_FIRST_ID, str, buffer, buffer_size, real);
		}
	}
	else
	{
		return this->predicate.randomAccess(id, str, buffer, buffer_size, real);
	}
}

void StringIndex::addRequest(std::vector<StringIndexFile::AccessRequest> *requestVectors, unsigned id, std::string *str, bool is_entity_or_literal)
{
	if (is_entity_or_literal)
	{
		//if(id == 9)
		//{
		//cout<<"to search 9 in string buffer"<<endl;
		//}
		//if(searchBuffer(id, str))
		//{
		////			*str = trie->Uncompress(*str)
		//cout<<"found in string buffer"<<endl;
		//return;
		//}
		if (id < Util::LITERAL_FIRST_ID) // entity
		{
			this->entity.addRequest(requestVectors[0], id, str);
		}	
		else // literal
		{
			this->literal.addRequest(requestVectors[1], id - Util::LITERAL_FIRST_ID, str);
		}	
	}
	else // predicate
	{
		this->predicate.addRequest(requestVectors[2], id, str);
	}
}

void StringIndex::trySequenceAccess(std::vector<StringIndexFile::AccessRequest>* requestVectors, char* &buffer, unsigned &buffer_size, bool real)
{
    latch.lockShared();
	this->entity.trySequenceAccess(requestVectors[0], buffer, buffer_size, real);
	this->literal.trySequenceAccess(requestVectors[1], buffer, buffer_size, real);
	this->predicate.trySequenceAccess(requestVectors[2], buffer, buffer_size, real);
    latch.unlock();
}

void StringIndex::SetTrie(Trie* trie) {
    return;
}

void StringIndex::change(std::vector<unsigned> &ids, KVstore &kv_store, bool is_entity_or_literal)
{
	latch.lockExclusive();
	if (is_entity_or_literal)
	{
		for (unsigned i = 0; i < ids.size(); i++)
		{
			if (ids[i] < Util::LITERAL_FIRST_ID)
				this->entity.change(ids[i], kv_store);
			else
				this->literal.change(ids[i] - Util::LITERAL_FIRST_ID, kv_store);
		}
	}
	else
	{
		for (unsigned i = 0; i < ids.size(); i++)
			this->predicate.change(ids[i], kv_store);
	}
	latch.unlock();
}

void StringIndex::disable(std::vector<unsigned> &ids, bool is_entity_or_literal)
{
	latch.lockExclusive();
	if (is_entity_or_literal)
	{
		for (unsigned i = 0; i < ids.size(); i++)
		{
			if (ids[i] < Util::LITERAL_FIRST_ID)
				this->entity.disable(ids[i]);
			else
				this->literal.disable(ids[i] - Util::LITERAL_FIRST_ID);
		}
	}
	else
	{
		for (unsigned i = 0; i < ids.size(); i++)
			this->predicate.disable(ids[i]);
	}
	latch.unlock();
}

