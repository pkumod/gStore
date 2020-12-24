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
#include "node/SINode.h"
#include "node/SIIntlNode.h"
#include "node/SILeafNode.h"
#include "storage/SIStorage.h"

//NOTICE:not use Stream for this tree, and no need for range query

class SITree {
  private:
  unsigned height; //0 indicates an empty tree
  SINode* root;
  SINode* leaves_head; //the head of LeafNode-list
  SINode* leaves_tail; //the tail of LeafNode-list
  std::string mode;    //BETTER(to use enum)
  SIStorage* TSM;      //Tree-Storage-Manage

  //always alloc one more byte than length, then user can add a '\0'
  //to get a real string, instead of new and copy
  //other operations will be harmful to search, so store value in
  //transfer temporally, while length adjusted.
  //TODO: in multi-user case, multiple-search will cause problem,
  //so lock is a must. Add lock to transfer is better than to add
  //lock to every key/value. However, modify requires a lock for a
  //key/value, and multiple search for different keys are ok!!!
  //Bstr transfer[3];	//0:transfer value searched; 1:copy key-data from const char*; 2:copy val-data from const char*
  //unsigned transfer_size[3];

  //TODO: in all B+ trees, updat eoperation should lock the whole tree, while search operations not
  //However, the transfer bstr maybe cause the parallism error!!!!
  //Why we need the transfer? It is ok to pass the original string pointer to return
  //A problem is that before the caller ends, the tree can not be modified(so a read-writ elock is required)

  std::string storepath;
  std::string filename; //ok for user to change
  /* some private functions */
  std::string getFilePath(); //in UNIX system
  //void CopyToTransfer(const char* _str, unsigned _len, unsigned _index);
  void release(SINode* _np) const;

  //tree's operations should be atom(if read nodes)
  //sum the request and send to Storage at last
  //ensure that all nodes operated are in memory
  long long request;
  void prepare(SINode* _np);

  std::mutex AccessLock;

  unsigned getHeight() const;
  void setHeight(unsigned _h);
  SINode* getRoot() const;
  SINode* find(const Bstr* _key, int* store, bool ifmodify);
  SINode* find(const char* _key, unsigned _len, int* store, bool ifmodify);

  public:
  SITree(); //always need to initial transfer
  SITree(std::string _storepath, std::string _filename, std::string _mode, unsigned long long _buffer_size);
  //insert, search, remove, set
  bool search(const char* _str, unsigned _len, unsigned* _val);
  bool insert(char* _str, unsigned _len, unsigned _val);
  bool modify(const char* _str, unsigned _len, unsigned _val);
  bool remove(const char* _str, unsigned _len);
  bool save();
  ~SITree();
  void print(std::string s); //DEBUG(print the tree)
};
//NOTICE: need to save tree manually before delete, otherwise will cause problem.
//(problem range between two extremes: not-modified, totally-modified)
//After saved, it's ok to continue operations on tree!

#endif
