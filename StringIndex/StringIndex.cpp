/*=============================================================================
# Filename: StringIndex.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-09-15
# Description: implement functions in StringIndex.h
=============================================================================*/

#include "StringIndex.h"

using namespace std;

void StringIndexFile::setNum(int _num)
{
	this->num = _num;
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

	fwrite(&this->num, sizeof(int), 1, this->index_file);

	long offset = 0;
	for (int i = 0; i < this->num; i++)
	{
		string str;
		if (this->type == Entity)
			str = kv_store.getEntityByID(i);
		if (this->type == Literal)
			str = kv_store.getLiteralByID(Util::LITERAL_FIRST_ID + i);
		if (this->type == Predicate)
			str = kv_store.getPredicateByID(i);

		int length = str.length();
		fwrite(&offset, sizeof(long), 1, this->index_file);
		fwrite(&length, sizeof(int), 1, this->index_file);
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

	fread(&this->num, sizeof(int), 1, this->index_file);

	this->index_table.resize(this->num);
	for (int i = 0; i < this->num; i++)
	{
		fread(&this->index_table[i].offset, sizeof(long), 1, this->index_file);
		fread(&this->index_table[i].length, sizeof(int), 1, this->index_file);
		this->empty_offset = max(this->empty_offset, this->index_table[i].offset + (long)this->index_table[i].length);
	}
}

bool StringIndexFile::randomAccess(int id, string *str)
{
	if (id < 0 || id >= this->num)
		return false;

	long offset = this->index_table[id].offset;
	int length = this->index_table[id].length;

	allocBuffer(length);

	fseek(this->value_file, offset, SEEK_SET);
	fread(this->buffer, sizeof(char), length, this->value_file);
	this->buffer[length] = '\0';

	*str = string(this->buffer);

	return true;
}

void StringIndexFile::addRequest(int id, std::string *str)
{
	this->request.push_back(AccessRequest(id, this->index_table[id].offset, this->index_table[id].length, str));
}

void StringIndexFile::trySequenceAccess()
{
	long max_end = 0;
	for (int i = 0; i < (int)this->request.size(); i++)
		max_end = max(max_end, this->request[i].offset + long(this->request[i].length));

	if (this->type == Entity)
		cout << "Entity StringIndex ";
	if (this->type == Literal)
		cout << "Literal StringIndex ";
	if (this->type == Predicate)
		cout << "Predicate StringIndex ";
	if (max_end / (long)1e5 < (long)this->request.size())
	{
		cout << "sequence access." << endl;

		sort(this->request.begin(), this->request.end());

		int pos = 0;
		fseek(this->value_file, 0, SEEK_SET);
		char *block = new char[MAX_BLOCK_SIZE];
		long current_block_begin = 0;
		while (current_block_begin < max_end)
		{
			long current_block_end = min(current_block_begin + MAX_BLOCK_SIZE, max_end);
			fread(block, sizeof(char), current_block_end - current_block_begin, this->value_file);

			while (pos < (int)this->request.size())
			{
				long offset = this->request[pos].offset;
				long length = this->request[pos].length;
				if (offset >= current_block_end)
					break;
				if (current_block_begin <= offset && offset + length <= current_block_end)
				{
					allocBuffer(length);
					memcpy(this->buffer, &block[offset - current_block_begin], length);
					this->buffer[length] = '\0';
					*this->request[pos].str = string(this->buffer);
					pos++;
				}
				else if (current_block_begin <= offset)
				{
					length = current_block_end - offset;
					allocBuffer(length);
					memcpy(this->buffer, &block[offset - current_block_begin], length);
					this->buffer[length] = '\0';
					*this->request[pos].str = string(this->buffer);
					break;
				}
				else if (offset + length <= current_block_end)
				{
					length = offset + length - current_block_begin;
					allocBuffer(length);
					memcpy(this->buffer, block, length);
					this->buffer[length] = '\0';
					*this->request[pos].str += string(this->buffer);
					pos++;
					while (pos < (int)this->request.size() && this->request[pos - 1].offset == this->request[pos].offset)
					{
						*this->request[pos].str = *this->request[pos - 1].str;
						pos++;
					}
				}
				else
				{
					length = current_block_end - current_block_begin;
					allocBuffer(length);
					memcpy(this->buffer, block, length);
					this->buffer[length] = '\0';
					*this->request[pos].str += string(this->buffer);
					break;
				}
			}

			current_block_begin = current_block_end;
		}
		delete[] block;
	}
	else
	{
		cout << "random access." << endl;

		for (int i = 0; i < (int)this->request.size(); i++)
			this->randomAccess(this->request[i].id, this->request[i].str);
	}
	this->request.clear();
}


void StringIndexFile::change(int id, KVstore &kv_store)
{
	if (id < 0)	return;

	if (this->num <= id)
	{
		while (this->num <= id)
		{
			this->index_table.push_back(IndexInfo());

			fseek(this->index_file, sizeof(int) + this->num * (sizeof(long) + sizeof(int)), SEEK_SET);
			fwrite(&this->index_table[this->num].offset, sizeof(long), 1, this->index_file);
			fwrite(&this->index_table[this->num].length, sizeof(int), 1, this->index_file);

			this->num++;
		}

		fseek(this->index_file, 0, SEEK_SET);
		fwrite(&this->num, sizeof(int), 1, this->index_file);
	}

	string str;
	if (this->type == Entity)
		str = kv_store.getEntityByID(id);
	if (this->type == Literal)
		str = kv_store.getLiteralByID(Util::LITERAL_FIRST_ID + id);
	if (this->type == Predicate)
		str = kv_store.getPredicateByID(id);

	this->index_table[id].offset = this->empty_offset;
	this->index_table[id].length = str.length();
	this->empty_offset += this->index_table[id].length;

	fseek(this->index_file, sizeof(int) + id * (sizeof(long) + sizeof(int)), SEEK_SET);
	fwrite(&this->index_table[id].offset, sizeof(long), 1, this->index_file);
	fwrite(&this->index_table[id].length, sizeof(int), 1, this->index_file);

	fseek(this->value_file, this->index_table[id].offset, SEEK_SET);
	fwrite(str.c_str(), sizeof(char), this->index_table[id].length, this->value_file);
}

void StringIndexFile::disable(int id)
{
	if (id < 0 || id >= this->num)	return ;

	this->index_table[id] = IndexInfo();

	fseek(this->index_file, sizeof(int) + id * (sizeof(long) + sizeof(int)), SEEK_SET);
	fwrite(&this->index_table[id].offset, sizeof(long), 1, this->index_file);
	fwrite(&this->index_table[id].length, sizeof(int), 1, this->index_file);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------
void StringIndex::setNum(StringIndexFile::StringIndexFileType _type, int _num)
{
	if (_type == StringIndexFile::Entity)
		this->entity.setNum(_num);
	if (_type == StringIndexFile::Literal)
		this->literal.setNum(_num);
	if (_type == StringIndexFile::Predicate)
		this->predicate.setNum(_num);
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
StringIndex::searchBuffer(int _id, string* _str)
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

bool StringIndex::randomAccess(int id, string *str, bool is_entity_or_literal)
{
	if(id < 0) return false;

	if (is_entity_or_literal)
	{
		if(searchBuffer(id, str))
		{
			return true;
		}

		if (id < Util::LITERAL_FIRST_ID)
			return this->entity.randomAccess(id, str);
		else
			return this->literal.randomAccess(id - Util::LITERAL_FIRST_ID, str);
	}
	else
	{
		return this->predicate.randomAccess(id, str);
	}
}

void StringIndex::addRequest(int id, std::string *str, bool is_entity_or_literal)
{
	if (is_entity_or_literal)
	{
		if(searchBuffer(id, str))
		{
			return;
		}
		if (id < Util::LITERAL_FIRST_ID)
			this->entity.addRequest(id, str);
		else
			this->literal.addRequest(id - Util::LITERAL_FIRST_ID, str);
	}
	else
	{
		this->predicate.addRequest(id, str);
	}
}

void StringIndex::trySequenceAccess()
{
	this->entity.trySequenceAccess();
	this->literal.trySequenceAccess();
	this->predicate.trySequenceAccess();
}

void StringIndex::change(std::vector<unsigned> &ids, KVstore &kv_store, bool is_entity_or_literal)
{
	if (is_entity_or_literal)
	{
		for (int i = 0; i < (int)ids.size(); i++)
		{
			if (ids[i] < Util::LITERAL_FIRST_ID)
				this->entity.change(ids[i], kv_store);
			else
				this->literal.change(ids[i] - Util::LITERAL_FIRST_ID, kv_store);
		}
	}
	else
	{
		for (int i = 0; i < (int)ids.size(); i++)
			this->predicate.change(ids[i], kv_store);
	}
}

void StringIndex::disable(std::vector<unsigned> &ids, bool is_entity_or_literal)
{
	if (is_entity_or_literal)
	{
		for (int i = 0; i < (int)ids.size(); i++)
		{
			if (ids[i] < Util::LITERAL_FIRST_ID)
				this->entity.disable(ids[i]);
			else
				this->literal.disable(ids[i] - Util::LITERAL_FIRST_ID);
		}
	}
	else
	{
		for (int i = 0; i < (int)ids.size(); i++)
			this->predicate.disable(ids[i]);
	}
}

