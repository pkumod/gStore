/*=============================================================================
# Filename: Node.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:38
# Description: basic Node class, father of IntlNode and LeafNode
=============================================================================*/

#ifndef _NODE_H
#define _NODE_H

#include "../util/Util.h"
#include "../bstr/TBstr.h"

class Node       				//abstract basic class 
{
public:
	static const unsigned DEGREE = 2 * 63;	//the degree of B+ tree
	static const unsigned MAX_CHILD_NUM = DEGREE;		
	static const unsigned MIN_CHILD_NUM = DEGREE >> 1;
	static const unsigned MAX_KEY_NUM = MAX_CHILD_NUM - 1;	//max key-num
	static const unsigned MIN_KEY_NUM = MIN_CHILD_NUM - 1;	//min key-num
	/* diffrent flags for tree-nodes, 32-bit put rank in low-bits means no need to move*/
	static const unsigned NF_IL = 0x80000000;	//is leaf
	static const unsigned NF_ID = 0x00080000;	//is dirty, in rank-area
	static const unsigned NF_IM = 0x20000000;	//in memory, not virtual
	//static const unsigned NF_IV = 0x10000000;	//is virtual
	static const unsigned NF_RK = 0x00ffffff;	//select-rank, in Storage
	static const unsigned NF_HT = 0xf00000;		//height area in rank
	static const unsigned NF_KN = 0x07f000;		//NOTICE: decided by DEGREE
	static const unsigned INTL_SIZE = sizeof(TBstr) * MAX_KEY_NUM;
	static const unsigned LEAF_SIZE = 2 * INTL_SIZE;
protected:
	unsigned store;			//store address, the BLock index
	unsigned flag;			//NF_RK, NF_IL,NF_ID, NF_IV, propety
	//int num; 			//totle keys num
	//Node* father;		//point to father-node, which must be IntlNode
	TBstr* keys; 
	void AllocKeys();
	//void FreeKeys();
public:
	Node();		
	Node(bool isVirtual);
	bool isLeaf() const;
	bool isDirty() const;
	void setDirty();
	void delDirty();
	bool inMem() const;
	void setMem();
	void delMem();
	//bool isVirtual() const;
	//void setVirtual();
	//void delVirtual();
	unsigned getRank() const;
	void setRank(unsigned _rank);
	unsigned getHeight() const;
	void setHeight(unsigned _h);
	unsigned getNum() const;
	bool setNum(int _num);
	bool addNum();
	bool subNum();
	unsigned getStore() const;
	void setStore(unsigned _store);
	unsigned getFlag() const;
	void setFlag(unsigned _flag);
	const TBstr* getKey(int _index) const;	//need to check the index
	bool setKey(const TBstr* _key, int _index, bool ifcopy = false);
	bool addKey(const TBstr* _key, int _index, bool ifcopy = false);
	bool subKey(int _index, bool ifdel = false);
	//virtual functions: polymorphic
	virtual Node* getChild(int _index) const { return NULL; };
	virtual bool setChild(Node* _child, int _index) { return true; };
	virtual bool addChild(Node* _child, int _index) { return true; };
	virtual bool subChild(int _index) { return true; };
	virtual Node* getPrev() const { return NULL; };
	virtual Node* getNext() const { return NULL; };
	virtual const TBstr* getValue(int _index) const { return NULL; };
	virtual bool setValue(const TBstr* _value, int _index, bool ifcopy = false) { return true; };
	virtual bool addValue(const TBstr* _value, int _index, bool ifcopy = false) { return true; };
	virtual bool subValue(int _index, bool ifdel = false) { return true;};
	virtual void setPrev(Node* _prev) {};
	virtual void setNext(Node* _next) {};
	virtual void Virtual() = 0;
	virtual void Normal() = 0;
	virtual unsigned getSize() const = 0;		//return all memory owned
	virtual Node* split(Node* _father, int _index) = 0;
	virtual Node* coalesce(Node* _father, int _index) = 0;
	virtual void release() = 0;		//release the node, only remain necessary information
	virtual ~Node() {};
	virtual void print(std::string s) = 0;		//DEBUG(print the Node)
};

/*NOTICE(operations in release()) 
 *To save memory, we can only remain store and flag(childs added for Leaf).
 *However, in this way childs'pointers is ok to change, use Node** or Node*& is also nonsense
 *because the pointer variable may die. 
 *Another way is only to release dynamic memory, and store thw whole, read the TBstr only to 
 *build. In this way nodes'pointer doesn't change, and operation is simplified, while memory
 *is consumed a bit more. Because TBstrs consume the most memory, and memory-disk swapping is
 *the most time-consuming thing, it seems to be a better way.
 *WARN:when a node is in memory and not deleted, its basic content is always being! If nodes are 
 *really too many, this will cause disaster because we can't swap them out until tree is closed!
 *To solve this problem, there should be two types of release-function: one to release TBstr, one 
 *to release the whole(pointer is invalid and rebuild problem)
 */

#endif

