/*=============================================================================
# Filename: VNode.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 14:05
# Description: written by liyouhuan
=============================================================================*/

#ifndef _VSTREE_VNODE_H
#define _VSTREE_VNODE_H

#include "../Util/Util.h"
#include "../Signature/SigEntry.h"
#include "LRUCache.h"

class VNode {
  public:
  static const int DEGREE = 100; //use 100 for normal
                                 //NOTICE+WARN:always ensure double times due to union operation safety in coalesce
                                 //here 201 is used to ensure split operation is right
                                 //(generally, 200 is ok if dealed carefully, but here only 200 is really used)
  static const int MAX_CHILD_NUM = 2 * VNode::DEGREE + 1;
  //static const int MAX_CHILD_NUM = 151;
  static const int MIN_CHILD_NUM = VNode::DEGREE;
  //static const int MIN_CHILD_NUM = 60;

  //size of Vnode
  static const int VNODE_SIZE = sizeof(int) * (3 + MAX_CHILD_NUM) + sizeof(SigEntry) * (MAX_CHILD_NUM + 1);

  //extract different parts of flag
  static const unsigned DIRTY_PART = 0x08000000;
  static const unsigned DEL_DIRTY_PART = ~DIRTY_PART;
  static const unsigned LEAF_PART = 0x04000000;
  static const unsigned DEL_LEAF_PART = ~LEAF_PART;
  static const unsigned ROOT_PART = 0x02000000;
  static const unsigned DEL_ROOT_PART = ~ROOT_PART;
  //NOTICE:child num is below 256, so use 8 bits is ok
  static const unsigned NUM_PART = 0x000000ff;
  static const unsigned DEL_NUM_PART = ~NUM_PART;

#ifdef THREAD_VSTREE_ON
  //NOTICE: rw lock has higher cost than mutex lock
  //mutex lock: if in memory
  pthread_mutex_t node_lock;
#endif

  //debug
  //    static const int MAX_CHILD_NUM = 50;
  //    static const int MIN_CHILD_NUM = 20;
  VNode();
  VNode(bool _is_leaf);
  ~VNode();

  void setFlag(unsigned _flag);

  bool isLeaf() const;
  bool isRoot() const;
  bool isFull() const;

  void setAsLeaf(bool _isLeaf);
  void setAsRoot(bool _isRoot);

  bool isDirty() const;
  void setDirty(bool _flag = true);

  int getChildNum() const;
  int getFileLine() const;
  int getFatherFileLine() const;
  int getChildFileLine(int _i) const;

  void setChildNum(int _num);
  void setFileLine(int _line);
  void setFatherFileLine(int _line);
  void setChildFileLine(int _i, int _line);

  const SigEntry& getEntry() const;
  const SigEntry& getChildEntry(int _i) const;
  void setEntry(const SigEntry _entry);
  void setChildEntry(int _i, const SigEntry _entry);

  VNode* getFather(LRUCache& _nodeBuffer) const;        // get the father node's pointer.
  VNode* getChild(int _i, LRUCache& _nodeBuffer) const; // get the _i-th child node's pointer.

  /* add one child node to this node. when splitting this node, can add a new child to it. */
  bool addChildNode(VNode* _p_child_node, bool _is_splitting = false);
  /* add one child entry to this node. when splitting this node, can add a new entry to it. */
  bool addChildEntry(const SigEntry _entry, bool _is_splitting = false);
  bool removeChild(int _i);

  int getIndexInFatherNode(LRUCache& _nodeBuffer);
  void refreshSignature();                              // just refresh itself signature.
  void refreshAncestorSignature(LRUCache& _nodeBuffer); // refresh self and its ancestor's signature.

  /* used by internal Node */
  bool retrieveChild(std::vector<VNode*>& _child_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer);
  /* only used by leaf Node */
  bool retrieveEntry(std::vector<SigEntry>& _entry_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer);

  //for debug
  bool checkState();

  std::string to_str();

  //NOTICE: read and write based on unequal IntlNode and LeafNode
  bool readNode(FILE* _fp);
  bool writeNode(FILE* _fp);

  private:
  //BETTER:including height and modify the memory-disk swap strategy
  //then is_root flag is unnecessary
  unsigned flag;
  //bool dirty;
  //bool is_leaf;
  //bool is_root;
  //int child_num;
  int self_file_line;
  int father_file_line;
  SigEntry entry;
  SigEntry child_entries[VNode::MAX_CHILD_NUM];

  //NOTICE: in leaf node, no need to keep the big child array
  //int child_file_lines[VNode::MAX_CHILD_NUM];
  int* child_file_lines;

  void AllocChilds();
  void InitLock();
};

#endif // _VSTREE_VNODE_H
