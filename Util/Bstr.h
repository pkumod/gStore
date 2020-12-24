/*=============================================================================
# Filename: Bstr.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-16 13:01
# Description: 
1. firstly written by liyouhuan, modified by zengli
2. class declaration for Bstr(used to store arbitary string) 
=============================================================================*/

#ifndef _UTIL_BSTR_H
#define _UTIL_BSTR_H

#include "Util.h"

class Bstr {
  private:
  char* str; //pointers consume 8 byte in 64-bit system
  //TODO: the length maybe not needed
  unsigned length;

  public:
  Bstr();
  //if copy memory, then use const char*, but slow
  //else, can not use const char* -> char*
  Bstr(const char* _str, unsigned _len, bool _nocopy = false);
  //Bstr(char* _str, unsigned _len);
  Bstr(const Bstr& _bstr);
  //Bstr& operate = (const Bstr& _bstr);

  bool operator>(const Bstr& _bstr);
  bool operator<(const Bstr& _bstr);
  bool operator==(const Bstr& _bstr);
  bool operator<=(const Bstr& _bstr);
  bool operator>=(const Bstr& _bstr);
  bool operator!=(const Bstr& _bstr);
  unsigned getLen() const;
  void setLen(unsigned _len);
  char* getStr() const;
  void setStr(char* _str); //reuse a TBstr
  void release();          //release memory
  void clear();            //set str/length to 0
  void copy(const Bstr* _bp);
  void copy(const char* _str, unsigned _len);
  //bool read(FILE* _fp);
  //int write(FILE* _fp);
  ~Bstr();
  void print(std::string s) const; //DEBUG

  //judge if this Bstr represent a long list value, and waiting to be each time on need
  bool isBstrLongList() const;
};

#endif // _UTIL_BSTR_H
