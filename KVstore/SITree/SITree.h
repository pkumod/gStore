/*=============================================================================
# Filename: SITree.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:44
# Description: string2ID, including entity2id, literal2id, predicate2id
=============================================================================*/

#ifndef _KVSTORE_SITREE_SITREE_H
#define _KVSTORE_SITREE_SITREE_H

#include "../../Util/Util.h"
#include "../../Util/Stream.h"
#include "../../Util/SpinLock.h"
#include "node/SINode.h"
#include "node/SIIntlNode.h"
#include "node/SILeafNode.h"
#include "storage/SIStorage.h"

/**
 * NOTICE:not use Stream for this tree, and no need for range query
 * This class is designed for storing 'string - ID' pairs and is used
 * when query begins.
 * The KVstore class have 3 private member : entity2id, literal2id, predicate2id,
 * whose type is SITree
 *
 * The SITree implemented a type of mixed B+ tree. The inner node part is B tree, while
 * the leaves part is B+ tree (a linked list).
 *
 * An example of SITree logic structure:
 * root                        [25| | ]
 *                         /           \
 *                       /              \
 * L1            [18|21| ]               [45| | ]
 *         /         |       \             |     \
 *      /           |          \           |      \
 * L2 [12|15| ]-[18|19|20]-[21|22|23]-[25|30|31]-[45|47|48|50]
 *
 */
class SITree
{					
private:
	unsigned height_;		//0 indicates an empty tree
	SINode* root_;
	SINode* leaves_head_;			//the head of LeafNode-list
	SINode* leaves_tail_;			//the tail of LeafNode-list
	std::string mode_;           //BETTER(to use enum)
	SIStorage* tsm_;           	//Tree-Storage-Manage
	bool if_single_thread; 
	//always alloc one more byte than length, then user can add a '\0'
	//to get a real string, instead of new and copy
	//other operations will be harmful to Search, so store value in
	//transfer temporally, while length adjusted.
	//TODO: in multi-user case, multiple-Search will cause problem,
	//so lock is a must. Add lock to transfer is better than to add 
	//lock to every key/value. However, modify requires a lock for a
	//key/value, and multiple Search for different keys are ok!!!
	//Bstr transfer[3];	//0:transfer value searched; 1:copy key-data from const char*; 2:copy val-data from const char*
	//unsigned transfer_size[3];

	//TODO: in all B+ trees, update operation should lock the whole tree, while Search operations not
	//However, the transfer bstr maybe cause the parallelism error!!!!
	//Why we need the transfer? It is ok to pass the original string pointer to return
	//A problem is that before the caller ends, the tree can not be modified(so a read-write lock is required)

	std::string store_path_;
	std::string filename_;      	//ok for user to change


	std::string GetFilePath();	//in UNIX system
	void Release(SINode* _np) const;

	//tree's operations should be atom(if read nodes)
	//sum the request and send to Storage at last
	//ensure that all nodes operated are in memory
	long long request_;
	void Prepare(SINode* _np);

	std::mutex access_lock_;

	unsigned GetHeight() const;
	void SetHeight(unsigned _h);
	SINode* GetRoot() const;
	SINode* Find(const char* _key, unsigned _len, int* store, bool if_modify);

public:
	SITree();				//always need to initial transfer
	SITree(std::string store_path, std::string _filename, std::string _mode, unsigned long long _buffer_size);

	bool Search(const char* _str, unsigned _len, unsigned* _val);
	bool Insert(char* str, unsigned len, unsigned val);
	bool Modify(const char* _str, unsigned _len, unsigned _val);
	bool Remove(const char* _str, unsigned _len);
	bool Save();

	//NOTICE: need to Save tree manually before delete, otherwise will cause problem.
    //(problem range between two extremes: not-modified, totally-modified)
    //After saved, it's ok to continue operations on tree!
	~SITree();
	void Print(std::string s);			//DEBUG(print the tree)
	void SetSingleThread(bool _single);
};


#endif

