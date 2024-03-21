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
#include "../Util/SpinLock.h"
#include "../Util/Latch.h"

class StringIndexFile
{
	public:
		//static const long MAX_BLOCK_SIZE = 10000000;
		static const long MAX_BLOCK_SIZE = 2097152;
		enum StringIndexFileType {Entity, Literal, Predicate};
		void SetTrie(Trie *trie);
	private:
		StringIndexFileType type;
		std::string loc;
		unsigned num;
		long empty_offset;
		FILE *index_file, *value_file;
		//never delete this->trie ,because this trie is the trie KVSTORE owns
		Trie *trie;
		class IndexInfo
		{
			public:
				long offset;
				unsigned length;
				IndexInfo(long _offset = 0, unsigned _length = 0):offset(_offset), length(_length){}
		};
		std::vector<IndexInfo>* index_table;
		// unsigned buffer_size;
		// char *buffer;
		// char *UncompressBuffer;
	public:

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
		// std::vector<AccessRequest> request;

		StringIndexFile(StringIndexFileType _type, std::string _dir, unsigned _num):type(_type), num(_num), empty_offset(0), index_file(NULL), value_file(NULL)
		{
			if (this->type == Entity)
				this->loc = _dir + "/entity_";
			if (this->type == Literal)
				this->loc = _dir + "/literal_";
			if (this->type == Predicate)
				this->loc = _dir + "/predicate_";
			this->index_table = new std::vector<IndexInfo>;
			// UncompressBuffer = new char[MAX_BLOCK_SIZE*4];
			//dictionary_path = _dir + "/../dictionary.dc";
			//trie = new Trie;
		}
		~StringIndexFile()
		{
			if (this->index_file != NULL)
				fclose(this->index_file);
			if (this->value_file != NULL)
				fclose(this->value_file);
			// if (this->buffer != NULL)
			// 	delete[] this->buffer;
			delete this->index_table;
			// delete[] UncompressBuffer;
//			if (this->trie != NULL)
//				delete trie;
		}

		void clear()
		{
			this->index_file = NULL;
			this->value_file = NULL;
			this->index_table = NULL;
//			this->trie = NULL;
		}

		void setNum(unsigned _num);
		unsigned getNum();
		void save(KVstore &kv_store);
		void load();
		inline void allocBuffer(char* &buffer, unsigned &buffer_size, unsigned length)
		{
			if (buffer_size <= length)
			{
				delete[] buffer;
				buffer_size = length + 1;
				buffer = new char[buffer_size];
			}
		}

		// inline void emptyBuffer()
		// {
		// 	this->buffer_size = 0;
		// 	this->buffer = NULL;
		// }

		bool randomAccess(unsigned id, std::string *str, char* &buffer, unsigned &buffer_size, bool real = true);

        inline void addRequest(std::vector<AccessRequest> &request, unsigned id, std::string *str)
        {
        	request.push_back(AccessRequest(id, (*this->index_table)[id].offset, (*this->index_table)[id].length, str));
        }

        void trySequenceAccess(std::vector<StringIndexFile::AccessRequest> request, char* &buffer, unsigned &buffer_size, bool real = true);

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
		
		//mutex AccessLock;
		// spinlock AccessLock;
		Latch latch;
		//atomic_flag spinlock = ATOMIC_FLAG_INIT ;
	public:
//		Trie *trie;
		void SetTrie(Trie* trie);
		StringIndex(std::string _dir, unsigned _entity_num = 0, unsigned _literal_num = 0, unsigned _predicate_num = 0):
			entity(StringIndexFile::Entity, _dir, _entity_num), literal(StringIndexFile::Literal, _dir, _literal_num), predicate(StringIndexFile::Predicate, _dir, _predicate_num)
		{
//			trie = entity.trie;
		}

		void clear()
		{
			entity.clear();
			literal.clear();
			predicate.clear();
		}

		void flush()
		{
			//nothing to do here
		}

		// void emptyBuffer()
		// {
		// 	entity.emptyBuffer();
		// 	literal.emptyBuffer();
		// 	predicate.emptyBuffer();
		// }

		void setBuffer(Buffer* _ebuf, Buffer* _lbuf)
		{
			this->entity_buffer = _ebuf;
			this->entity_buffer_size = _ebuf->size;
			this->literal_buffer = _lbuf;
			this->literal_buffer_size = _lbuf->size;
		}
		bool searchBuffer(unsigned _id, std::string* _str);

		void setNum(StringIndexFile::StringIndexFileType _type, unsigned _num);
		unsigned getNum(StringIndexFile::StringIndexFileType _type);

		void save(KVstore &kv_store);
		void load();

		bool randomAccess(unsigned id, std::string *str, char* &buffer, unsigned &buffer_size, bool is_entity_or_literal = true, bool real = true);
        void addRequest(std::vector<StringIndexFile::AccessRequest> *requestVectors, unsigned id, std::string *str, bool is_entity_or_literal = true);
        void trySequenceAccess(std::vector<StringIndexFile::AccessRequest>* requestVectors, char* &buffer, unsigned &buffer_size, bool real = true);

		void change(std::vector<unsigned> &ids, KVstore &kv_store, bool is_entity_or_literal = true);
		void disable(std::vector<unsigned> &ids, bool is_entity_or_literal = true);
};

#endif // _STRING_INDEX_H
