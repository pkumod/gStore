/*=============================================================================
# Filename: LeafNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: the leaf-node of a B+ tree
=============================================================================*/

#ifndef _LEAF_NODE_H
#define _LEAF_NODE_H

#include "../util/Util.h"
#include "../bstr/TBstr.h"
#include "Node.h"

class LeafNode: public Node    
{
protected:
	Node* prev;	//LeafNode
	Node* next;
	TBstr* values;
	void AllocValues();
	//void FreeValues();
public:
	LeafNode();
	LeafNode(bool isVirtual);
	//LeafNode(Storage* TSM);
	void Virtual();
	void Normal();
	Node* getPrev() const;
	Node* getNext() const;
	const TBstr* getValue(int _index) const;	
	bool setValue(const TBstr* _value, int _index, bool ifcopy = false);
	bool addValue(const TBstr* _value, int _index, bool ifcopy = false);
	bool subValue(int _index, bool ifdel = false);
	void setPrev(Node* _prev);
	void setNext(Node* _next);
	unsigned getSize() const;
	Node* split(Node* _father, int _index);
	Node* coalesce(Node* _father, int _index);
	void release();			
	~LeafNode();
	void print(std::string s);			//DEBUG
	/*non-sense virtual function
	Node* getChild(int _index) const;
	bool addChild(Node* _child, int _index);
	bool subChild(int _index);
	*/
};
//BETTER: prev isn't a must, and reverse-range can be achieved using recursive-next

#endif

