/*=============================================================================
# Filename: RangeValue.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:42
# Description: achieve functions in RangeValue.h
=============================================================================*/

#include "RangeValue.h"

using namespace std;

RangeValue::RangeValue()
{
  this->fp = NULL;
  this->transfer.setStr((char*)malloc(1 << 20));
  this->transfer_size = 1 << 20;
}

void
RangeValue::reset()
{
  fseek(this->fp, 0, SEEK_SET);
}

bool
RangeValue::open()
{
  if (this->fp != NULL)
    fclose(this->fp);
  if ((this->fp = fopen("logs/answer.dat", "w+b")) == NULL)
    return false;
  else
    return true;
}

bool
RangeValue::write(const Bstr* _bp)
{
  if (this->fp == NULL)
    return false;
  unsigned len = _bp->getLen();
  fwrite(&len, sizeof(unsigned), 1, this->fp);
  fwrite(_bp->getStr(), sizeof(char), len, this->fp);
  return true;
}

const Bstr*
RangeValue::read()
{
  if (this->fp == NULL)
    return NULL;
  unsigned len = 0;
  fread(&len, sizeof(unsigned), 1, this->fp);
  if (feof(this->fp))
    return NULL; //indicate the end
  if (len + 1 > this->transfer_size) {
    transfer.release();
    transfer.setStr((char*)malloc(len + 1));
    this->transfer_size = len + 1;
  }
  fread(transfer.getStr(), sizeof(char), len, this->fp);
  transfer.getStr()[len] = '\0'; //set for string() in KVstore
  transfer.setLen(len);
  return &transfer;
}

RangeValue::~RangeValue()
{
  if (this->fp != NULL)
    fclose(this->fp);
  //transfer will de deleted as Bstr
}
