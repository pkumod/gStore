/*=============================================================================
# Filename:	Stream.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 14:10
# Description: achieve functions in Stream.h
=============================================================================*/

#include "Stream.h"

using namespace std;

ResultCmp mycmp;

//DEBUG: error when using STL::sort() to sort the Bstr[] units with mycmp, null pointer(Bstr*)
//reported sometimes(for example, watdiv_30.db and watdiv_200.db, query/C3.sql).
//Notice that sort() uses quick-sorting method when size is large, which usually
//performs faster than merge-sorting used by STL::stable_sort() which can ensures the order between same
//value(only in the sorted column) units.
//The error is marked by DEBUG1 and DEBUG2, and I just use STL::stable_sort() here, because I cannot find
//the reason of the null pointer error if using STL::sort()

void
Stream::init()
{
  this->inMem = true;
  this->mode = -1;
  this->ansMem = NULL;
  this->ansDisk = NULL;
  this->rownum = this->colnum = 0;
  this->needSort = false;
  this->xpos = this->ypos = 0; //the 0-th pos is not used now
  this->record = NULL;
  this->record_size = NULL;
  this->space = 0;
  this->tempfp = NULL;
}

Stream::Stream()
{
  this->init();
}

Stream::Stream(std::vector<TYPE_ENTITY_LITERAL_ID>& _keys, std::vector<bool>& _desc, unsigned _rownum, unsigned _colnum, bool _flag)
{
  this->init();
#ifdef DEBUG_STREAM
  printf("Stream:now to open stream\n");
#endif

  this->rownum = _rownum;
  this->colnum = _colnum;
  this->needSort = _flag;
  //this->cmp = ResultCmp(this->rownum, _keys);
  mycmp = ResultCmp(this->rownum, _keys, _desc);

  this->record = new Bstr[this->colnum];
  this->record_size = new unsigned[this->colnum];
  for (unsigned i = 0; i < this->colnum; ++i) {
    char* tmptr = new char[Util::TRANSFER_SIZE];
    this->record[i].setStr(tmptr);
    this->record_size[i] = Util::TRANSFER_SIZE;
  }

  this->mode = 0; //wait for writing records

  long long size = (long long)_rownum * (long long)_colnum * 100 / Util::GB;
  //TODO: get this arg from memory manager
  if (Util::memoryLeft() < size) {
    this->inMem = false;
    fprintf(stderr, "Stream: memory is not enough!\n");
  } else {
    this->inMem = true;
    fprintf(stderr, "Stream: memory is enough!\n");
  }
#ifdef DEBUG_STREAM
  fprintf(stderr, "Stream:after memory check!\n");
#endif

#ifdef DEBUG_STREAM
  fprintf(stderr, "Stream::Stream() - basic information\n");
  fprintf(stderr, "rownum: %u\tcolnum: %u\n", this->rownum, this->colnum);
  if (this->needSort) {
    fprintf(stderr, "the result needs to be sorted, the keys are listed below:\n");
    for (vector<int>::iterator it = _keys.begin(); it != _keys.end(); ++it)
      fprintf(stderr, "%d\t", *it);
    fprintf(stderr, "\n");
  } else {
    fprintf(stderr, "the result needs not to be sorted!\n");
  }
//WARN: this is just for debugging!
//this->inMem = false;
#endif

  if (this->inMem) {
    this->ansMem = new Bstr* [this->rownum];
    for (unsigned i = 0; i < this->rownum; ++i) {
      this->ansMem[i] = new Bstr[this->colnum];
    }
    return;
  }

  //below are for disk
  if (!this->needSort) // in disk and need sort
  {
    string file_name = Util::tmp_path + "thread_" + Util::getThreadID() + "_" + Util::int2string(Util::get_cur_time());
    file_name += ".dat";
#ifdef DEBUG_STREAM
    fprintf(stderr, "%s\n", file_name.c_str());
#endif
    //FILE* fp = NULL;
    if ((this->ansDisk = fopen(file_name.c_str(), "w+b")) == NULL) {
      fprintf(stderr, "Stream::Stream(): open error!\n");
      return;
    }
    this->result = file_name;
  }
  //return true;
}

bool operator<(const Element& _a, const Element& _b)
{
  return mycmp(_a.val, _b.val);
}

bool operator>(const Element& _a, const Element& _b)
{
  return !mycmp(_a.val, _b.val);
}

bool
Stream::copyToRecord(const char* _str, unsigned _len, unsigned _idx)
{
  if (_idx >= this->colnum) {
    fprintf(stderr, "Stream::copyToRecord: index out of range!\n");
    return false;
  }

  unsigned length = _len;
  if (length + 1 > this->record_size[_idx]) {
    this->record[_idx].release();
    char* tmptr = new char[length + 1];
    this->record[_idx].setStr(tmptr);
    this->record_size[_idx] = length + 1; //one more byte: convenient to add \0
  }

  memcpy(this->record[_idx].getStr(), _str, length);
  this->record[_idx].getStr()[length] = '\0'; //set for string() in KVstore
  this->record[_idx].setLen(length);
  return true;
}

void
Stream::outputCache()
{
//DEBUG1
//sort and output to file
#ifndef PARALLEL_SORT
  stable_sort(this->tempst.begin(), this->tempst.end(), mycmp);
#else
  omp_set_num_threads(thread_num);
  __gnu_parallel::stable_sort(this->tempst.begin(), this->tempst.end(), mycmp);
#endif
  unsigned size = this->tempst.size();
  for (unsigned i = 0; i < size; ++i) {
    Bstr* p = this->tempst[i];
    for (unsigned j = 0; j < this->colnum; ++j) {
      unsigned len = p[j].getLen();
      char* str = p[j].getStr();
      fwrite(&len, sizeof(unsigned), 1, this->tempfp);
      fwrite(str, sizeof(char), len, this->tempfp);
    }
    delete[] p;
  }
  this->tempst.clear();

  //reset and add to heap, waiting for merge sort
  fseek(this->tempfp, 0, SEEK_SET);
  Bstr* bp = new Bstr[this->colnum];

  for (unsigned i = 0; i < this->colnum; ++i) {
    unsigned len;
    fread(&len, sizeof(unsigned), 1, this->tempfp);
    //char* p = (char*)malloc(len * sizeof(char));
    char* p = new char[len];
    fread(p, sizeof(char), len, this->tempfp);
    bp[i].setLen(len);
    bp[i].setStr(p);
  }
  this->sortHeap.push_back(Element(this->tempfp, bp));
  this->tempfp = NULL;
  this->space = 0;
}

bool
Stream::write(const char* _str, unsigned _len)
{
#ifdef DEBUG_PRECISE
  fprintf(stderr, "Stream::write(): the current column is %u\n", this->ypos);
#endif
  this->copyToRecord(_str, _len, this->ypos);
  this->ypos++;
  if (this->ypos == this->colnum) {
    this->ypos = 0;
#ifdef DEBUG_PRECISE
    fprintf(stderr, "Stream::write(): now a record is ready, the current row is %u\n", this->xpos);
#endif
    return this->write(this->record);
  }
  return true;
}

bool
Stream::write(const Bstr* _bp)
{
  if (this->xpos >= this->rownum) {
    fprintf(stderr, "you should set the end now!\n");
    return false;
  }

  if (this->inMem) {
    //Bstr** p = (Bstr**)this->ans;
    for (unsigned i = 0; i < this->colnum; ++i) {
      //this->ansMem[this->xpos][i].release();
      this->ansMem[this->xpos][i].copy(_bp + i);
    }
    this->xpos++;
    return true;
  }

  //below are for disk
  if (needSort) //NOTICE:in disk and need sort
  {
    if (this->tempfp == NULL) {
      string name = Util::tmp_path + "thread_" + Util::getThreadID() + "_stream_" + Util::int2string(Util::get_cur_time());
      name += ".dat";
#ifdef DEBUG_STREAM
      fprintf(stderr, "%s\n", name.c_str());
#endif
      if ((this->tempfp = fopen(name.c_str(), "w+b")) == NULL) {
        fprintf(stderr, "Stream::write(): open error!\n");
        return false;
      }
      this->files.push_back(name);
    }

    Bstr* p = new Bstr[this->colnum];
    for (unsigned i = 0; i < this->colnum; ++i) {
      //p[i].release();
      p[i].copy(_bp + i);
      this->space += _bp->getLen();
    }
    this->space += sizeof(unsigned) * this->colnum;
    this->space += sizeof(char*) * this->colnum;
    this->tempst.push_back(p);
    this->xpos++;

    if (this->space > Stream::BASE_MEMORY_LIMIT) {
      this->outputCache();
    }
  } else {
    //FILE* fp = (FILE*)(this->ans);
    for (unsigned i = 0; i < this->colnum; ++i) {
      unsigned len = _bp[i].getLen();
      const char* str = _bp[i].getStr();
      fwrite(&len, sizeof(unsigned), 1, this->ansDisk);
      fwrite(str, sizeof(char), len, this->ansDisk);
    }
    this->xpos++;
  }

  return true;
}

const Bstr*
Stream::read()
{
  if (this->isEnd()) {
    fprintf(stderr, "read to end now!\n");
    return NULL;
  }

  if (this->inMem) {
    //Bstr** bp = (Bstr**)(this->ans);
    Bstr* ip = this->ansMem[this->xpos];
    for (unsigned i = 0; i < this->colnum; ++i) {
      this->copyToRecord(ip[i].getStr(), ip[i].getLen(), i);
      //this->record[i].release();
      //unsigned len = ip[i].getLen();
      //char* s = (char*)calloc(len + 1, sizeof(char));
      //memcpy(s, ip[i].getStr(), len);
      //this->record[i].setLen(len);
      //this->record[i].setStr(s);
    }
  } else {
    //below are for disk, both needSort and not
    //FILE* fp = (FILE*)(this->ans);
    for (unsigned i = 0; i < this->colnum; ++i) {
      //BETTER:alloc and reuse the space in Bstr?
      unsigned len;
      fread(&len, sizeof(unsigned), 1, this->ansDisk);
      //char* s = (char*)calloc(len + 1, sizeof(char));
      char* s = new char[len + 1];
      fread(s, sizeof(char), len, this->ansDisk);
      s[len] = '\0';
      this->copyToRecord(s, len, i);
      //free(s);
      delete[] s;
    }
  }
  this->xpos++;
  if (this->xpos == this->rownum)
    this->mode = 2;
  return this->record;

  //if(feof((FILE*)this->fp))
  //return NULL;		//indicate the end
  //unsigned len = 0;
  //fread(&len, sizeof(unsigned), 1, (FILE*)this->fp);
  //if(len + 1 > this->transfer_size)
  //{
  //transfer.release();
  //transfer.setStr((char*)malloc(len+1));
  //this->transfer_size = len + 1;
  //}
  //fread(transfer.getStr(), sizeof(char), len, (FILE*)this->fp);
  //transfer.getStr()[len] = '\0';	//set for string() in KVstore
  //transfer.setLen(len);
  //return &transfer;
}

bool
Stream::isEnd()
{
  return this->mode == 2;
}

//do multi-list merge sort using heap
void
Stream::mergeSort()
{
  string file_name = Util::tmp_path + Util::int2string(Util::get_cur_time());
  file_name += ".dat";
#ifdef DEBUG_STREAM
  fprintf(stderr, "%s\n", file_name.c_str());
#endif
  //FILE* fp = NULL;
  if ((this->ansDisk = fopen(file_name.c_str(), "w+b")) == NULL) {
    fprintf(stderr, "Stream::mergeSort: open error!\n");
    return;
  }

  unsigned valid = this->sortHeap.size();
  vector<Element>::iterator begin = this->sortHeap.begin();
  make_heap(begin, begin + valid, greater<Element>());
  while (valid > 0) {
#ifdef DEBUG_STREAM
    fprintf(stderr, "valid: %u\n", valid);
#endif
    //write contents of the first element to result file
    Bstr* bp = this->sortHeap[0].val;
    for (unsigned i = 0; i < this->colnum; ++i) {
      unsigned len = bp[i].getLen();
      char* s = bp[i].getStr();
#ifdef DEBUG_STREAM
      fprintf(stderr, "top %u: %u\n", i, len);
      for (unsigned j = 0; j < len; ++j)
        fprintf(stderr, "%c", s[j]);
      fprintf(stderr, "\n");
#endif
      fwrite(&len, sizeof(unsigned), 1, this->ansDisk);
      fwrite(s, sizeof(char), len, this->ansDisk);
      bp[i].release();
    }
#ifdef DEBUG_STREAM
    fprintf(stderr, "\n");
#endif

    //pop, read and adjust
    pop_heap(begin, begin + valid, greater<Element>());
    bp = this->sortHeap[valid - 1].val;
    bool tillEnd = false;
    for (unsigned i = 0; i < this->colnum; ++i) {
      unsigned len;
      char* s;
      FILE* tp = this->sortHeap[valid - 1].fp;
      fread(&len, sizeof(unsigned), 1, tp);
      if (feof(tp)) {
        this->sortHeap[valid - 1].release();
        valid--;
        tillEnd = true;
#ifdef DEBUG_STREAM
        fprintf(stderr, "now a stream file reaches its end!\n");
#endif
        break;
      }

      //s = (char*)malloc(sizeof(char) * len);
      s = new char[len];
      fread(s, sizeof(char), len, tp);
      bp[i].setLen(len);
      bp[i].setStr(s);
    }
    if (!tillEnd)
      push_heap(begin, begin + valid, greater<Element>());
  }

  //fseek(fp, 0, SEEK_SET);
  //this->ans = fp;
  this->result = file_name;
}

void
Stream::setEnd()
{
  if (this->mode == 1) {
    fprintf(stderr, "Stream::setEnd(): already in read mode!\n");
    this->xpos = 0;
    //FILE* fp = (FILE*)(this->ans);
    if (!this->inMem)
      fseek(this->ansDisk, 0, SEEK_SET);
    return;
  }

  this->mode = 1; //wait for reading records
  this->xpos = 0;

#ifdef DEBUG_STREAM
  fprintf(stderr, "Stream::setEnd(): now is in read mode!\n");
#endif

  if (this->inMem) {
    //Bstr** p = (Bstr**)(this->ans);
    if (this->needSort) {
//DEBUG2
#ifndef PARALLEL_SORT
      stable_sort(this->ansMem, this->ansMem + this->rownum, mycmp);
#else
      omp_set_num_threads(thread_num);
      __gnu_parallel::stable_sort(this->ansMem, this->ansMem + this->rownum, mycmp);
#endif
    }
    return;
  }

  //below are for disk
  if (this->needSort) {
    if (this->tempfp != NULL) {
      this->outputCache();
    }
    if (this->files.size() > 1) {
#ifdef DEBUG_STREAM
      fprintf(stderr, "Stream::setEnd(): merge sort is needed here!\n");
#endif
      //do multi-list merge sort using heap
      this->mergeSort();
    } else if (this->files.size() > 0) //==1
    {
      this->sortHeap[0].release();
      this->ansDisk = fopen(this->files[0].c_str(), "r+b");
      this->result = this->files[0];
    }
  }
  //FILE* fp = (FILE*)(this->ans);
  fseek(this->ansDisk, 0, SEEK_SET);
}

Stream::~Stream()
{
  delete[] this->record;
  delete[] this->record_size;
#ifdef DEBUG_STREAM
  fprintf(stderr, "Stream::~Stream(): record deleted!\n");
#endif

  if (this->inMem) {
    //Bstr** bp = (Bstr**)(this->ans);
    for (unsigned i = 0; i < this->rownum; ++i) {
      delete[] this->ansMem[i];
      //bp[i] = NULL;
    }
    delete[] this->ansMem;
#ifdef DEBUG_STREAM
    fprintf(stderr, "Stream::~Stream(): in memory, now table deleted!\n");
#endif
    //TODO:memory leak -- how about tempst
    return;
  }

  //below are for disk, both needSort and not
  //FILE* fp = (FILE*)(this->ans);
  fclose(this->ansDisk);

  //remove files and result
  remove(this->result.c_str());
  for (vector<string>::iterator it = this->files.begin(); it != this->files.end(); ++it)
    remove((*it).c_str());
#ifdef DEBUG_STREAM
  fprintf(stderr, "Stream::~Stream(): in disk, now all files removed!\n");
#endif

  //#ifdef DEBUG_PRECISE
  //printf("file is closed in Stream!\n");
  //#endif
}
