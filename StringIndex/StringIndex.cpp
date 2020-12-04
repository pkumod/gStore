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
string  StringIndexFile::get_loc()
{
	return this->loc;
}
long StringIndexFile::GetOffsetbyID(unsigned _id)
{
	return (*this->index_table)[_id].offset;
}

long StringIndexFile::GetLengthbyID(unsigned _id)
{
	return (*this->index_table)[_id].length;
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
	cout << "String index save num=" << num << endl;
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

bool StringIndexFile::randomAccess(unsigned id, string *str, bool real)
{
	if (id >= this->num)
		return false;

	long offset = (*this->index_table)[id].offset;
	unsigned length = (*this->index_table)[id].length;
	//if(id == 9)
	//{
	//cout<<"check: "<<offset<<" "<<length<<endl;
	//}

	allocBuffer(length);

	//DEBUG: here a bug exists if we use pread instead of fread, the details are in BUG_StringIndex_pread of docs/BUGS.md
	fseek(this->value_file, offset, SEEK_SET);
	fread(this->buffer, sizeof(char), length, this->value_file);
	//pread(fileno(value_file), this->buffer, sizeof(char)*length, offset);
	this->buffer[length] = '\0';

	if (real)
		trie->Uncompress(this->buffer, length, *str, this->UncompressBuffer);
	else
		*str = buffer;

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


void* StringIndexFile::thread_read(void * argv)
{
	int begin = (int)*(long*)argv;
	int end = (int)*((long*)argv + 1);
	StringIndexFile *thisp=(StringIndexFile *)(*((long*)argv + 2));
	long offset, length;
	unsigned id;
	char *Mmap = thisp->Mmap;
	char *Buffer = new char[MAX_BLOCK_SIZE * 8];
	string *base = thisp->base;
	for (int pos = begin; pos <= end; pos++)
	{
		id = thisp->request[pos].id;
		offset = (*thisp->index_table)[id].offset;
		length = (*thisp->index_table)[id].length;
		thisp->trie->Uncompress(&Mmap[offset], length, *(base+ thisp->request[pos].off_str), Buffer);
	}
	delete[] Buffer;
	return NULL;
}

void StringIndexFile::trySequenceAccess(bool real, pthread_t tidp)
{
	long t0 = Util::get_cur_time();
	if (this->request.empty())
		return;

	int requestsize = (int)this->request.size();
	unsigned minid = this->request[0].id;
	unsigned maxid = 0;
	for (int i = 0; i < requestsize; i++)
	{
		if (minid > this->request[i].id)
			minid = this->request[i].id;
		if (maxid < this->request[i].id)
			maxid = this->request[i].id;
	}

	long min_begin = (*this->index_table)[minid].offset;
	long max_end = (*this->index_table)[maxid].offset + (*this->index_table)[maxid].length;

	/*
	long min_begin = this->request[0].offset; 
	long max_end = 0;

	int requestsize = (int)this->request.size();
	//cout << "size=" << requestsize << endl;

	for (int i = 0; i < requestsize; i++)
	{
		if (min_begin > this->request[i].offset)
			min_begin = this->request[i].offset;
		if (max_end < this->request[i].offset + long(this->request[i].length))
			max_end = this->request[i].offset + long(this->request[i].length);
	}
	*/
	if (this->type == Entity)
		cout << "Entity StringIndex ";
	if (this->type == Literal)
		cout << "Literal StringIndex ";
	if (this->type == Predicate)
		cout << "Predicate StringIndex ";

	if ((max_end - min_begin) / 800000L < (long)this->request.size())
	{
		cout << "sequence access." << endl;
		
#ifndef PARALLEL_SORT
		sort(this->request.begin(), this->request.end());
#else
		omp_set_num_threads(thread_num);
		__gnu_parallel::sort(this->request.begin(), this->request.end());
#endif
		if (tidp != (pthread_t) -1)
		{
			pthread_join(tidp, NULL);
			cout << "after get sort and wait for thread used " << Util::get_cur_time() - t0 << " ms" << endl;
		}
		else
			cout << "after get sort  used " << Util::get_cur_time() - t0 << "ms" << endl;
		long t1 = Util::get_cur_time();


		long offset, length;
		unsigned id;
		char *Mmap = this->Mmap;
		string *base = this->base;

		for (int pos = 0; pos < requestsize; pos++)
		{
			id = this->request[pos].id;
			offset = (*this->index_table)[id].offset;
			length = (*this->index_table)[id].length;
			this->trie->Uncompress(&Mmap[offset], length, *(base + this->request[pos].off_str), UncompressBuffer);
		}
	

		/*
		this is the abondomed code for multi threads
		int p_num = 4;
		if ((int)this->request.size()  < 1000000)
			p_num = 1;
		cout << "thread num:" << p_num << endl;
		long arg[p_num][3];
		long loc = 0;
		long step = (int)this->request.size() / p_num;
		for (int i = 0; i < p_num; i++)
		{
			arg[i][0] = loc;
			if (i != p_num - 1)
				arg[i][1] = loc + step - 1;
			else
				arg[i][1] = ((int)this->request.size() - 1);
			arg[i][2] = (long)this;
			loc += step;
		}
		pthread_t tidp[p_num];
		for (int i = 0; i <p_num; i++)
			pthread_create(&tidp[i], NULL, &StringIndexFile::thread_read, (void*)arg[i]);
		for (int i = 0; i<p_num; i++)
			pthread_join(tidp[i], NULL);
		*/
		cout << "after thread read used " << Util::get_cur_time() - t1 << " ms" << endl;
	}
	else
	{
		cout << "random access." << endl;
		for (int i = 0; i < (int)this->request.size(); i++)
			this->randomAccess(this->request[i].id, (this->request[i].off_str + base), real);
	}
	this->request.clear();
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
	fwrite(str.c_str(), sizeof(char), (*this->index_table)[id].length , this->value_file);
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
	if (_id < Util::LITERAL_FIRST_ID) //entity
	{
		if (_id < this->entity_buffer_size)
		{
			*_str = this->entity_buffer->get(_id);
			return true;
		}
		return false;
	}
	else //literal
	{
		_id -= Util::LITERAL_FIRST_ID;
		if (_id < this->literal_buffer_size)
		{
			*_str = this->literal_buffer->get(_id);
			return true;
		}
		return false;
	}
}

bool StringIndex::randomAccess(unsigned id, string *str, bool is_entity_or_literal, bool real)
{
	if (id < 0) return false;

	if (is_entity_or_literal)
	{
		//if(searchBuffer(id, str))
		//{
		//cout << "FLAG2" << endl;
		//*str = trie->Uncompress(*str, str->length());
		//return true;
		//}

		if (id < Util::LITERAL_FIRST_ID)
		{
			return this->entity.randomAccess(id, str, real);
		}
		else
		{
			return this->literal.randomAccess(id - Util::LITERAL_FIRST_ID, str, real);
		}
	}
	else
	{
		return this->predicate.randomAccess(id, str, real);
	}
}
void 
StringIndex::addRequest(unsigned id, unsigned off_base, bool is_entity_or_literal)
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
		if (id < Util::LITERAL_FIRST_ID)
			this->entity.addRequest(id, off_base);
		else
			this->literal.addRequest(id - Util::LITERAL_FIRST_ID, off_base);
	}
	else
	{
		this->predicate.addRequest(id, off_base);
	}
}

void StringIndex::trySequenceAccess(bool real, pthread_t tidp)
{
	AccessLock.lock();
	this->entity.trySequenceAccess(real,tidp);
	this->literal.trySequenceAccess(real, tidp);
	this->predicate.trySequenceAccess(real, tidp);
	AccessLock.unlock();
}

void StringIndex::SetTrie(Trie* trie)
{
	AccessLock.lock();
	this->entity.SetTrie(trie);
	this->literal.SetTrie(trie);
	this->predicate.SetTrie(trie);
	AccessLock.unlock();
}

vector<StringIndexFile*>
StringIndex::get_three_StringIndexFile()
{
	vector<StringIndexFile*> ret;
	ret.push_back(&this->entity);
	ret.push_back(&this->literal);
	ret.push_back(&this->predicate);
	return ret;
}

void StringIndex::change(std::vector<unsigned> &ids, KVstore &kv_store, bool is_entity_or_literal)
{
	AccessLock.lock();
	if (is_entity_or_literal)
	{
		if (this->entity.mmapLength != 0)
			munmap(this->entity.Mmap, this->entity.mmapLength);
		if (this->literal.mmapLength != 0)
			munmap(this->literal.Mmap, this->literal.mmapLength);

		for (unsigned i = 0; i < ids.size(); i++)
		{
			if (ids[i] < Util::LITERAL_FIRST_ID)
				this->entity.change(ids[i], kv_store);
			else
				this->literal.change(ids[i] - Util::LITERAL_FIRST_ID, kv_store);
		}
		this->entity.flush_file();
		this->literal.flush_file();
	}
	else
	{
		if (this->predicate.mmapLength != 0)
			munmap(this->predicate.Mmap, this->predicate.mmapLength);
		for (unsigned i = 0; i < ids.size(); i++)
			this->predicate.change(ids[i], kv_store);
		this->predicate.flush_file();
	}
	AccessLock.unlock();
}

void StringIndex::disable(std::vector<unsigned> &ids, bool is_entity_or_literal)
{
	AccessLock.lock();
	if (is_entity_or_literal)
	{
		if (this->entity.mmapLength != 0)
			munmap(this->entity.Mmap, this->entity.mmapLength);
		if (this->literal.mmapLength != 0)
			munmap(this->literal.Mmap, this->literal.mmapLength);

		for (unsigned i = 0; i < ids.size(); i++)
		{
			if (ids[i] < Util::LITERAL_FIRST_ID)
				this->entity.disable(ids[i]);
			else
				this->literal.disable(ids[i] - Util::LITERAL_FIRST_ID);
		}
		this->entity.flush_file();
		this->literal.flush_file();
	}
	else
	{
		if (this->predicate.mmapLength != 0)
			munmap(this->predicate.Mmap, this->predicate.mmapLength);
		for (unsigned i = 0; i < ids.size(); i++)
			this->predicate.disable(ids[i]);
		this->predicate.flush_file();
	}
	AccessLock.unlock();
}

