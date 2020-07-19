/*=============================================================================
# Filename: IVTree.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:44
# Description: ID2valueList, including s2po, p2so and o2ps
=============================================================================*/

#ifndef _KVSTORE_IVTREE_IVTREE_H
#define _KVSTORE_IVTREE_IVTREE_H

#include "../../Util/Util.h"
#include "../../Util/Stream.h"
#include "../../Util/VList.h"
#include "node/IVNode.h"
#include "node/IVIntlNode.h"
#include "node/IVLeafNode.h"
#include "storage/IVStorage.h"

//TODO: for long list, do not read in time, just on need
//the memory is kept with the node, updat ewith node
//NOTICE: to release the node, maybe the value list is NULL
//value bstr: unsigned=address, NULL
//BETTER?: build a new block store for long list??

//NOTICE: we do not need to use transfer bstr here, neithor for two directions
//when insert/query, we do not release the value in kvstore

class IVTree {
  protected:
  unsigned height; //0 indicates an empty tree
  IVNode* root;
  IVNode* leaves_head; //the head of LeafNode-list
  IVNode* leaves_tail; //the tail of LeafNode-list
  std::string mode;    //BETTER(to use enum)
  IVStorage* TSM;      //Tree-Storage-Manage
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
  //Bstr transfer;
  //unsigned transfer_size;
  //Bstr transfer[3];	//0:transfer value searched; 1:copy key-data from const char*; 2:copy val-data from const char*
  //unsigned transfer_size[3];

  //tree's operations should be atom(if read nodes)
  //sum the request and send to IVStorage at last
  //ensure that all nodes operated are in memory
  long long request;
  void prepare(IVNode* _np);

  std::string storepath;
  std::string filename;      //ok for user to change
                             /* some private functions */
  std::string getFilePath(); //in UNIX system
  //void CopyToTransfer(const char* _str, unsigned _len, unsigned _index);
  //void CopyToTransfer(const char* _str, unsigned _len);
  void release(IVNode* _np) const;

  //very long value list are stored in a separate file(with large block)
  //
  //NOTICE: according to the summary result, 90% value lists are just below 100 bytes
  //<10%: 5000000~100M bytes
  VList* value_list;

  public:
  IVTree(); //always need to initial transfer
  IVTree(std::string _storepath, std::string _filename, std::string _mode, unsigned long long _buffer_size);
  unsigned getHeight() const;
  void setHeight(unsigned _h);
  IVNode* getRoot() const;
  //void setRoot(Node* _root);
  //insert, search, remove, set
  bool search(unsigned _key, char*& _str, unsigned& _len);
  bool insert(unsigned _key, char* _str, unsigned _len);
  bool modify(unsigned _key, char* _str, unsigned _len);
  IVNode* find(unsigned _key, int* store, bool ifmodify);
  bool remove(unsigned _key);
  const Bstr* getRangeValue();
  void resetStream();
  bool range_query(unsigned _key1, unsigned _key2);
  bool save();

  void AddIntoCache(TYPE_PREDICATE_ID _id);
  void AddIntoCache(TYPE_ENTITY_LITERAL_ID _id);
  ~IVTree();
  void print(std::string s); //DEBUG(print the tree)
};
//NOTICE: need to save tree manually before delete, otherwise will cause problem.
//(problem range between two extremes: not-modified, totally-modified)
//After saved, it's ok to continue operations on tree!

#endif
