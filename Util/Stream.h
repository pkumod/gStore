/*=============================================================================
# Filename: Stream.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 13:18
# Description: 
1. stream buffer for medium results, store/write one record at a time
2. dynamicly change method(memory/disk) according to the memory usage of system
3. each Stream instance is asociated with one result-like object, and one file
4. functions using this class should implement writeToStream... to operate on different records
5. the records should be viewed as tables to deal with
=============================================================================*/

#ifndef _UTIL_STREAM_H
#define _UTIL_STREAM_H

#include "Util.h"
#include "Bstr.h"

//BETTER: use vector instead of table with fixed size, no need to indicate the rownum first(colnum required)
//It is really a question to use Bstr[] or string[] to store a record

//struct Stream::ResultEqual
//{
//    int result_len;
//    ResultEqual(int _l):result_len(_l){}
//    bool operator() (Bstr* const& a, Bstr* const& b)
//    {
//        for (int i = 0; i < result_len; ++i)
//        {
//            if (a[i] != b[i])
//				return false;
//        }
//        return true;
//    }
//};

struct ResultCmp {
  int result_len;
  std::vector<TYPE_ENTITY_LITERAL_ID> keys;
  std::vector<bool> desc;
  //ResultCmp(int _l):result_len(_l){}
  ResultCmp()
  {
    this->result_len = 0;
  }
  ResultCmp(int _l, std::vector<TYPE_ENTITY_LITERAL_ID>& _keys, std::vector<bool>& _desc)
  {
    this->result_len = _l;
    this->keys = std::vector<TYPE_ENTITY_LITERAL_ID>(_keys);
    this->desc = std::vector<bool>(_desc);
  }
  bool operator()(Bstr* const& a, Bstr* const& b)
  {
    //for(int i = 0; i < result_len; ++i)
    //{
    //if (a[i] != b[i])
    //return (a[i] < b[i]);
    //}
    unsigned size = this->keys.size();
    for (unsigned i = 0; i < size; ++i) {
      int t = this->keys[i];
      if (a[t] != b[t]) {
        if (!this->desc[i])
          return (a[t] < b[t]);
        else
          return (a[t] > b[t]);
      }
    }
    return true;
  }
};

//static ResultCmp mycmp;

typedef struct StreamElement {
  FILE* fp;
  Bstr* val;
  StreamElement(FILE* _fp, Bstr* _val)
  {
    this->fp = _fp;
    this->val = _val;
  }
  void release()
  {
    delete[] this->val;
    this->val = NULL;
    fclose(this->fp);
    this->fp = NULL;
  }
} Element;

//static bool operator < (const Element& _a, const Element& _b);

//BETTER:use mmap part by part to get output

//NOTICE:new and delete the Stream when you use it to store a series of result
//duplicates should not be considered here, because sort based on int-int is faster
//(so easy to remove duplicates)
//However, for 'order by', the string comparision is a must, which should be done here!
//(maybe in memory , maybe internal-external)
class Stream {
  private:
  //multi-way merge sort is used here to do the internal-external sort
  std::vector<Element> sortHeap;
  std::vector<std::string> files;
  FILE* tempfp;
  std::vector<Bstr*> tempst;
  unsigned space; //space used in disk for one file

  //struct ResultCmp cmp;

  //void* ans;               //FILE* if in disk, Bstr** if in memory
  Bstr** ansMem;
  FILE* ansDisk;
  std::string result; //needed if stored in disk, to be removed later
  unsigned rownum, colnum;
  bool needSort;
  //std::vector<int> keys;
  int mode; //-1:invalid;0:only write;1:only read;2:read end
  bool inMem;
  //below are for record position
  unsigned xpos, ypos;
  Bstr* record; //one record for read, array of Bstrs
  unsigned* record_size;

  void init();
  bool copyToRecord(const char* _str, unsigned _len, unsigned _idx);
  void outputCache();
  void mergeSort();

  public:
  //NOTICE:max num of opened files is 1024 in Linux by default, but this is enough for a result
  //as large as 1T
  static const unsigned BASE_MEMORY_LIMIT = 1 << 30;

  Stream();
  Stream(std::vector<TYPE_ENTITY_LITERAL_ID>& _keys, std::vector<bool>& _desc, unsigned _rownum, unsigned _colnum, bool _flag);

  //read/write should be based on the unit of record

  //NOTICE:this function copy/save the data, caller free the memory if needed
  bool write(const Bstr* _bp);
  bool write(const char* _str, unsigned _len);
  //NOTICE:the memory should not be freed by user, and the latter will flush the former!
  const Bstr* read();
  void setEnd();
  bool isEnd();
  ~Stream();
};

#endif //_UTIL_STREAM_H
