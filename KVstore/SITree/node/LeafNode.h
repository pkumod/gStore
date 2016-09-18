/*=============================================================================
# Filename: LeafNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: the leaf-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_SINODE_LEAFNODE_H
#define _KVSTORE_SINODE_LEAFNODE_H

#include "Node.h"

class SILeafNode: public SINode    
{
protected:
	SINode* prev;	//LeafNode
	SINode* next;
	int* values;
	void AllocValues();
	//void FreeValues();
public:
	SILeafNode();
	SILeafNode(bool isVirtual);
	//LeafNode(Storage* TSM);
	void Virtual();
	void Normal();
	SINode* getPrev() const;
	SINode* getNext() const;
	int getValue(int _index) const;	
	bool setValue(int _val, int _index);
	bool addValue(int _val, int _index);
	bool subValue(int _index);
	void setPrev(SINode* _prev);
	void setNext(SINode* _next);
	unsigned getSize() const;
	SINode* split(SINode* _father, int _index);
	SINode* coalesce(SINode* _father, int _index);
	void release();			
	~SILeafNode();
	void print(std::string s);			//DEBUG
	/*non-sense virtual function
	Node* getChild(int _index) const;
	bool addChild(Node* _child, int _index);
	bool subChild(int _index);
	*/
};
//BETTER: prev isn't a must, and reverse-range can be achieved using recursive-next

#endif

