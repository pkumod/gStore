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

class StringIndexFile
{
	public:
		static const long MAX_BLOCK_SIZE = 10000000;
		enum StringIndexFileType {Entity, Literal, Predicate};
	private:
		StringIndexFileType type;
		std::string loc;
		unsigned num;
		long empty_offset;
		FILE *index_file, *value_file;

		class IndexInfo
		{
			public:
				long offset;
				unsigned length;
				IndexInfo(long _offset = 0, unsigned _length = 0):offset(_offset), length(_length){}
		};
		std::vector<IndexInfo> index_table;

		unsigned buffer_size;
		char *buffer;

		class AccessRequest
		{
			public:
				unsigned id;
				long offset;
				unsigned length;
				std::string *str;
				AccessRequest(unsigned _id, long _offset, unsigned _length, std::string *_str):
					id(_id), offset(_offset), length(_length), str(_str){};
				inline bool operator < (const AccessRequest &x) const
				{
					return this->offset < x.offset;
				}
		};
		std::vector<AccessRequest> request;

	public:
		StringIndexFile(StringIndexFileType _type, std::string _dir, unsigned _num):type(_type), num(_num), empty_offset(0), index_file(NULL), value_file(NULL), buffer_size(0), buffer(NULL)
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
		void setNum(unsigned _num);

		void save(KVstore &kv_store);
		void load();

		inline void allocBuffer(unsigned length)
		{
			if (this->buffer_size <= length)
			{
				delete[] this->buffer;
				this->buffer_size = length + 1;
				this->buffer = new char[this->buffer_size];
			}
		}

		bool randomAccess(unsigned id, std::string *str);
		inline void addRequest(unsigned id, std::string *str)
		{
			this->request.push_back(AccessRequest(id, this->index_table[id].offset, this->index_table[id].length, str));
		}
		void trySequenceAccess();

		void change(unsigned id, KVstore &kv_store);
		void disable(unsigned id);
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
		StringIndex(std::string _dir, unsigned _entity_num = 0, unsigned _literal_num = 0, unsigned _predicate_num = 0):
			entity(StringIndexFile::Entity, _dir, _entity_num), literal(StringIndexFile::Literal, _dir, _literal_num), predicate(StringIndexFile::Predicate, _dir, _predicate_num){}

		void setBuffer(Buffer* _ebuf, Buffer* _lbuf)
		{
			this->entity_buffer = _ebuf;
			this->entity_buffer_size = _ebuf->size;
			this->literal_buffer = _lbuf;
			this->literal_buffer_size = _lbuf->size;
		}
		bool searchBuffer(unsigned _id, std::string* _str);

		void setNum(StringIndexFile::StringIndexFileType _type, unsigned _num);

		void save(KVstore &kv_store);
		void load();

		bool randomAccess(unsigned id, std::string *str, bool is_entity_or_literal = true);
		void addRequest(unsigned id, std::string *str, bool is_entity_or_literal = true);
		void trySequenceAccess();

		void change(std::vector<unsigned> &ids, KVstore &kv_store, bool is_entity_or_literal = true);
		void disable(std::vector<unsigned> &ids, bool is_entity_or_literal = true);
};

#endif // _STRING_INDEX_H
