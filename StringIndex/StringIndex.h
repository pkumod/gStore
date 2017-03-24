/*=============================================================================
# Filename: StringIndex.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-09-15
# Description:
=============================================================================*/

#ifndef _STRING_INDEX_H
#define _STRING_INDEX_H

#include "../KVstore/KVstore.h"
#include "../Util/Util.h"

//TODO: adjust the type

class StringIndexFile
{
	public:
		static const long MAX_BLOCK_SIZE = 10000000;
		enum StringIndexFileType {Entity, Literal, Predicate};
	private:
		StringIndexFileType type;
		std::string loc;
		int num;
		long empty_offset;
		FILE *index_file, *value_file;

		class IndexInfo
		{
			public:
				IndexInfo(long _offset = 0, int _length = 0):offset(_offset), length(_length){}
				long offset;
				int length;
		};
		std::vector<IndexInfo> index_table;

		int buffer_size;
		char *buffer;

		class AccessRequest
		{
			public:
				int id;
				long offset;
				int length;
				std::string *str;
				AccessRequest(int _id, long _offset, int _length, std::string *_str):
					id(_id), offset(_offset), length(_length), str(_str){};
				inline bool operator < (const AccessRequest &x) const
				{
					return this->offset < x.offset;
				}
		};
		std::vector<AccessRequest> request;

	public:
		StringIndexFile(StringIndexFileType _type, std::string _dir, int _num):type(_type), num(_num), empty_offset(0), index_file(NULL), value_file(NULL),  buffer_size(0), buffer(NULL)
		{
			if (this->type == Entity)
				this->loc = _dir + "/entity_";
			if (this->type == Literal)
				this->loc = _dir + "/literal_";
			if (this->type == Predicate)
				this->loc = _dir + "/predicate_";
		}
		~StringIndexFile()
		{
			if (this->index_file != NULL)
				fclose(this->index_file);
			if (this->value_file != NULL)
				fclose(this->value_file);
			if (this->buffer != NULL)
				delete[] this->buffer;
		}
		void setNum(int _num);

		void save(KVstore &kv_store);
		void load();

		inline void allocBuffer(int length)
		{
			if (this->buffer_size <= length)
			{
				delete[] this->buffer;
				this->buffer_size = length + 1;
				this->buffer = new char[this->buffer_size];
			}
		}

		bool randomAccess(int id, std::string *str);
		void addRequest(int id, std::string *str);
		void trySequenceAccess();

		void change(int id, KVstore &kv_store);
		void disable(int id);
};

class StringIndex
{
	private:
		StringIndexFile entity, literal, predicate;
		Buffer* entity_buffer;
		unsigned entity_buffer_size;
		Buffer* literal_buffer;
		unsigned literal_buffer_size;
	public:
		StringIndex(std::string _dir, int _entity_num = 0, int _literal_num = 0, int _predicate_num = 0) :
			entity(StringIndexFile::Entity, _dir, _entity_num), literal(StringIndexFile::Literal, _dir, _literal_num), predicate(StringIndexFile::Predicate, _dir, _predicate_num){}

		void setBuffer(Buffer* _ebuf, Buffer* _lbuf)
		{
			this->entity_buffer = _ebuf;
			this->entity_buffer_size = _ebuf->size;
			this->literal_buffer = _lbuf;
			this->literal_buffer_size = _lbuf->size;
		}
		bool searchBuffer(int _id, std::string* _str);

		void setNum(StringIndexFile::StringIndexFileType _type, int _num);

		void save(KVstore &kv_store);
		void load();

		bool randomAccess(int id, std::string *str, bool is_entity_or_literal = true);
		void addRequest(int id, std::string *str, bool is_entity_or_literal = true);
		void trySequenceAccess();

		void change(std::vector<unsigned> &ids, KVstore &kv_store, bool is_entity_or_literal = true);
		void disable(std::vector<unsigned> &ids, bool is_entity_or_literal = true);
};

#endif // _STRING_INDEX_H
