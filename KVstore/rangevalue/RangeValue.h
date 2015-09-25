/*=============================================================================
# Filename: RangeValue.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:41
# Description: set and deal of ranging values
=============================================================================*/

#ifndef _RANGE_VALUE_H
#define _RANGE_VALUE_H
#endif

#include "../util/Util.h"
#include "../bstr/TBstr.h"

class RangeValue
{
private:
	FILE* fp;
	TBstr transfer;
	unsigned transfer_size;
public:
	RangeValue();
	void reset();
	bool open();
	bool write(const TBstr* _bp);
	const TBstr* read();
	~RangeValue();
};

