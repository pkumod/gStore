/*=============================================================================
# Filename: Tree.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:44
# Description: struct and interface of the B+ tree
=============================================================================*/

#ifndef _KVSTORE_SITREE_TREE_H
#define _KVSTORE_SITREE_TREE_H

#include "../../Util/Util.h"
#include "../../Util/Stream.h"
#include "node/Node.h"
#include "node/IntlNode.h"
#include "node/LeafNode.h"
#include "storage/Storage.h"

//NOTICE:not use Stream for this tree, and no need for range query 

class SITree
{					
private:
	unsigned int height;		//0 indicates an empty tree
	SINode* root;
	SINode* leaves_head;			//the head of LeafNode-list
	SINode* leaves_tail;			//the tail of LeafNode-list
	std::string mode;           //BETTER(to use enum)
	SIStorage* TSM;           	//Tree-Storage-Manage

	//always alloc one more byte than length, then user can add a '\0'
	//to get a real string, instead of new and copy
	//other operations will be harmful to search, so store value in
	//transfer temporally, while length adjusted.
	//TODO: in multi-user case, multiple-search will cause problem,
	//so lock is a must. Add lock to transfer is better than to add 
	//lock to every key/value. However, modify requires a lock for a
	//key/value, and multiple search for different keys are ok!!!
	Bstr transfer[3];	//0:transfer value searched; 1:copy key-data from const char*; 2:copy val-data from const char*
	unsigned transfer_size[3];
	std::string storepath;
	std::string filename;      	//ok for user to change
	/* some private functions */
	std::string getFilePath();	//in UNIX system
	void CopyToTransfer(const char* _str, unsigned _len, unsigned _index);
	void release(SINode* _np) const;

	//tree's operations should be atom(if read nodes)
	//sum the request and send to Storage at last
	//ensure that all nodes operated are in memory
	long long request;
	void prepare(SINode* _np);

public:
	SITree();				//always need to initial transfer
	SITree(std::string _storepath, std::string _filename, std::string _mode);
	unsigned int getHeight() const;
	void setHeight(unsigned _h);
	SINode* getRoot() const;
	//insert, search, remove, set
	bool search(const char* _str, unsigned _len, int* _val);
	bool insert(const char* _str, unsigned _len, int _val);
	bool modify(const char* _str, unsigned _len, int _val);
	SINode* find(const Bstr* _key, int* store, bool ifmodify);
	bool remove(const char* _str, unsigned _len);
	bool save(); 			
	~SITree();
	void print(std::string s);			//DEBUG(print the tree)
};
//NOTICE: need to save tree manually before delete, otherwise will cause problem. 
//(problem range between two extremes: not-modified, totally-modified)
//After saved, it's ok to continue operations on tree!

#endif

