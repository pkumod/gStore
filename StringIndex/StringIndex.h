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

class StringIndexFile {
  public:
  //static const long MAX_BLOCK_SIZE = 10000000;
  static const long MAX_BLOCK_SIZE = 2097152;
  enum StringIndexFileType {
    Entity,
    Literal,
    Predicate
  };
  void SetTrie(Trie* trie);
  string get_loc();
  long GetOffsetbyID(unsigned _id);
  long GetLengthbyID(unsigned _id);
  char* Mmap;
  long mmapLength;

  private:
  StringIndexFileType type;
  std::string loc;
  unsigned num;
  long empty_offset;
  FILE* index_file, *value_file;
  //never delete this->trie ,because this trie is the trie KVSTORE owns
  Trie* trie;
  class IndexInfo {
public:
    long offset;
    unsigned length;
    IndexInfo(long _offset = 0, unsigned _length = 0)
        : offset(_offset)
        , length(_length)
    {
    }
  };
  std::vector<IndexInfo>* index_table;

  unsigned buffer_size;

  char* buffer;
  char* UncompressBuffer;

  class AccessRequest {
public:
    unsigned id;
    //long offset;
    //unsigned length;
    //std::string *str;
    unsigned off_str;
    /*AccessRequest(unsigned _id, long _offset, unsigned _length, std::string *_str):
					id(_id), offset(_offset), length(_length), str(_str){};*/
    //AccessRequest(unsigned _id, std::string *_str):
    //id(_id), str(_str){};
    AccessRequest(unsigned _id, unsigned _off_str)
        : id(_id)
        , off_str(_off_str) {};
    inline bool operator<(const AccessRequest& x) const
    {
      return this->id < x.id;
    }
    /*
				inline bool operator < (const AccessRequest &x) const
				{
					return this->offset < x.offset;
				}*/
  };
  std::vector<AccessRequest> request;

  //		std::string dictionary_path;
  string* base;

  public:
  void flush_file()
  {
    if (this->value_file != NULL)
      fclose(this->value_file);

    this->value_file = fopen((this->loc + "value").c_str(), "rb+");

    struct stat statbuf;
    int fd;
    stat((this->get_loc() + "value").c_str(), &statbuf);
    fd = open((this->get_loc() + "value").c_str(), O_RDONLY);
    this->mmapLength = (statbuf.st_size / 4096 + 1) * 4096;
    this->Mmap = (char*)mmap(NULL, this->mmapLength, PROT_READ, MAP_POPULATE | MAP_SHARED, fd, 0);
    close(fd);
  }
  void set_string_base(string* _base)
  {
    this->base = _base;
  }
  //Trie *trie;
  void request_reserve(int capcity)
  {
    request.reserve(capcity);
  }

  StringIndexFile(StringIndexFileType _type, std::string _dir, unsigned _num)
      : type(_type)
      , num(_num)
      , empty_offset(0)
      , index_file(NULL)
      , value_file(NULL)
      , buffer_size(0)
      , buffer(NULL)
  {
    if (this->type == Entity)
      this->loc = _dir + "/entity_";
    if (this->type == Literal)
      this->loc = _dir + "/literal_";
    if (this->type == Predicate)
      this->loc = _dir + "/predicate_";
    this->index_table = new std::vector<IndexInfo>;
    Mmap = NULL;
    mmapLength = 0;
    UncompressBuffer = new char[MAX_BLOCK_SIZE * 4];
    //dictionary_path = _dir + "/../dictionary.dc";
    //trie = new Trie;
  }
  ~StringIndexFile()
  {
    if (this->index_file != NULL)
      fclose(this->index_file);
    if (this->value_file != NULL)
      fclose(this->value_file);
    if (this->buffer != NULL)
      delete[] this->buffer;
    delete this->index_table;
    if (mmapLength != 0)
      munmap(Mmap, mmapLength);
    delete[] UncompressBuffer;
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
  void save(KVstore& kv_store);
  void load();
  static void* thread_read(void* argv);
  inline void allocBuffer(unsigned length)
  {
    if (this->buffer_size <= length) {
      delete[] this->buffer;
      this->buffer_size = length + 1;
      this->buffer = new char[this->buffer_size];
    }
  }

  inline void emptyBuffer()
  {
    this->buffer_size = 0;
    this->buffer = NULL;
  }

  bool randomAccess(unsigned id, std::string* str, bool real = true);

  inline void addRequest(unsigned id, unsigned off_base)
  {
    this->request.push_back(AccessRequest(id, off_base));
  }

  void trySequenceAccess(bool real = true, pthread_t tidp = -1);

  void change(unsigned id, KVstore& kv_store);
  void disable(unsigned id);
};

class StringIndex {
  private:
  StringIndexFile entity, literal, predicate;
  Buffer* entity_buffer;
  unsigned entity_buffer_size;
  Buffer* literal_buffer;
  unsigned literal_buffer_size;

  public:
  //		Trie *trie;
  void SetTrie(Trie* trie);
  vector<StringIndexFile*> get_three_StringIndexFile();
  StringIndex(std::string _dir, unsigned _entity_num = 0, unsigned _literal_num = 0, unsigned _predicate_num = 0)
      : entity(StringIndexFile::Entity, _dir, _entity_num)
      , literal(StringIndexFile::Literal, _dir, _literal_num)
      , predicate(StringIndexFile::Predicate, _dir, _predicate_num)
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

  void emptyBuffer()
  {
    entity.emptyBuffer();
    literal.emptyBuffer();
    predicate.emptyBuffer();
  }

  void setBuffer(Buffer* _ebuf, Buffer* _lbuf)
  {
    this->entity_buffer = _ebuf;
    this->entity_buffer_size = _ebuf->size;
    this->literal_buffer = _lbuf;
    this->literal_buffer_size = _lbuf->size;
  }
  bool searchBuffer(unsigned _id, std::string* _str);

  void setNum(StringIndexFile::StringIndexFileType _type, unsigned _num);

  void save(KVstore& kv_store);
  void load();

  bool randomAccess(unsigned id, std::string* str, bool is_entity_or_literal = true, bool real = true);
  void addRequest(unsigned id, unsigned off_base, bool is_entity_or_literal = true);
  void trySequenceAccess(bool real = true, pthread_t tidp = -1);
  void change(std::vector<unsigned>& ids, KVstore& kv_store, bool is_entity_or_literal = true);
  void disable(std::vector<unsigned>& ids, bool is_entity_or_literal = true);
};

#endif // _STRING_INDEX_H
