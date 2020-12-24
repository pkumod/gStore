/*=============================================================================
# Filename: RangeValue.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:41
# Description: set and deal of ranging values
=============================================================================*/

#ifndef _KVSTORE_RANGEVALUE_RANGEVALUE_H
#define _KVSTORE_RANGEVALUE_RANGEVALUE_H

#include "../../Util/Util.h"
#include "../../Bstr/Bstr.h"

class RangeValue {
  private:
  FILE* fp;
  Bstr transfer;
  unsigned transfer_size;

  public:
  RangeValue();
  void reset();
  bool open();
  bool write(const Bstr* _bp);
  const Bstr* read();
  ~RangeValue();
};

#endif
