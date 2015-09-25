/*=============================================================================
# Filename: Hash.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:36
# Description: to utilize the search for Node*	NOT USED YET
=============================================================================*/

#ifndef _HASH_H
#define _HASH_H

#include "../util/Util.h"
#include "../node/Node.h"

class Hash
{
private:
	Node* np;
	unsigned position;
	unsigned next;
	unsigned size;
public:
	Hash();
	Hash(unsigned _size);
};

#endif

