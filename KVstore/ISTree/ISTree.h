/*=============================================================================
# Filename: ISTree.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:44
# Description: ID2string, including id2entity, id2literal and id2predicate
=============================================================================*/

#ifndef _KVSTORE_ISTREE_ISTREE_H
#define _KVSTORE_ISTREE_ISTREE_H

#include "../../Util/Util.h"
#include "../../Util/Stream.h"
#include "node/ISNode.h"
#include "node/ISIntlNode.h"
#include "node/ISLeafNode.h"
#include "storage/ISStorage.h"

class ISTree {
  protected:
  unsigned height; //0 indicates an empty tree
  ISNode* root;
  ISNode* leaves_head; //the head of LeafNode-list
  ISNode* leaves_tail; //the tail of LeafNode-list
  std::string mode;    //BETTER(to use enum)
  ISStorage* TSM;      //Tree-Storage-Manage
                       //BETTER:multiple stream maybe needed:)
  Stream* stream;

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

  //tree's operations should be atom(if read nodes)
  //sum the request and send to ISStorage at last
  //ensure that all nodes operated are in memory
  long long request;
  void prepare(ISNode* _np);

  std::string storepath;
  std::string filename;      //ok for user to change
                             /* some private functions */
  std::string getFilePath(); //in UNIX system
  //void CopyToTransfer(const char* _str, unsigned _len, unsigned _index);
  void release(ISNode* _np) const;

  public:
  ISTree(); //always need to initial transfer
  ISTree(std::string _storepath, std::string _filename, std::string _mode, unsigned long long _buffer_size);
  unsigned getHeight() const;
  void setHeight(unsigned _h);
  ISNode* getRoot() const;
  //void setRoot(Node* _root);
  //insert, search, remove, set
  bool search(unsigned _key, char*& _str, unsigned& _len);
  bool insert(unsigned _key, char* _str, unsigned _len);
  bool modify(unsigned _key, char* _str, unsigned _len);
  ISNode* find(unsigned _key, int* store, bool ifmodify);
  bool remove(unsigned _key);
  const Bstr* getRangeValue();
  void resetStream();
  bool range_query(unsigned _key1, unsigned _key2);
  bool save();
  ~ISTree();
  void print(std::string s); //DEBUG(print the tree)
};
//NOTICE: need to save tree manually before delete, otherwise will cause problem.
//(problem range between two extremes: not-modified, totally-modified)
//After saved, it's ok to continue operations on tree!

#endif
