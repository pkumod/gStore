/*=============================================================================
# Filename: SINode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:38
# Description: basic SINode class, father of SIIntlNode and SILeafNode
=============================================================================*/

#ifndef _KVSTORE_SITREE_NODE_SINODE_H
#define _KVSTORE_SITREE_NODE_SINODE_H

#include "../../../Util/Util.h"
#include "../../../Util/Bstr.h"

/**
 * A virtual class. It haas two derived class: SIIntINode and SILeafNode
 * The basic unit of SITree. Defined as node in B+ tree. It compress node
 * information (dirty / in memory etc. )into 4 bytes. And its physical
 * address 'store' occupies another 4 bytes.
 *
 * The flag info is defined in some static const unsigned variable;
 * notice that NF_RK = 0x00ffffff, means that the latter 24 bit is used as
 * rank info.  The 0-3 bits of NF_RK is NF_HT, and the 4 bit is dirty bit.
 * The 5-11 bit records the children num. So, only the last 12 bits can be changed.
 *
 * In SIIntINode, suppose we have k keys, then we should have k+1 children
 * keys[i] is defined as the min key of the subtree rooted at child[i+1].
 * (Notice: min key of the subtree is not equal to the min key of SINode child[i+1] )
 * So in a SIIntINode si_node, if you want to find a  where a string t should be in,
 * You should do a binary search in si_node's keys and find a keys[i] <= t with keys[i+1] > t
 * Then t is in subtree rooted at child[i+1].
 */
class SINode       				//abstract basic class 
{
 public:
  static const unsigned DEGREE = 2 * 63;	//the degree of B+ tree
  static const unsigned MAX_CHILD_NUM = DEGREE;
  static const unsigned MIN_CHILD_NUM = DEGREE >> 1;
  static const unsigned MAX_KEY_NUM = MAX_CHILD_NUM - 1;	//max key-num
  static const unsigned MIN_KEY_NUM = MIN_CHILD_NUM - 1;	//min key-num

  // different flags for tree-nodes, 32-bit put rank in low-bits means no need to move
  static const unsigned NF_IL = 0x80000000;	//is leaf
  static const unsigned NF_ID = 0x00080000;	//is dirty, in rank-area
  static const unsigned NF_IM = 0x20000000;	//in memory, not virtual
  static const unsigned NF_RK = 0x00ffffff;	//select-rank, in Storage
  static const unsigned NF_HT = 0x00f00000;	//height area in rank
  static const unsigned NF_KN = 0x0007f000;	//NOTICE: decided by DEGREE
  static const unsigned INTL_SIZE = sizeof(Bstr) * MAX_KEY_NUM;
  int heapId;
  static const unsigned LEAF_SIZE = sizeof(unsigned) * MAX_KEY_NUM + INTL_SIZE;

 protected:
  //store address, the Block index
  unsigned store;
  unsigned node_flag_;			//NF_RK, NF_IL,NF_ID, NF_IV, propety
  Bstr* keys;
  void AllocKeys();
 public:
  SINode();
  SINode(bool isVirtual);
  // virtual ~SINode() { delete[] keys, keys = nullptr;};
  virtual ~SINode() { };

  bool isLeaf() const;
  bool isDirty() const;
  void setDirty();
  void delDirty();
  bool inMem() const;
  void SetInMem();
  void SetMemFlagFalse();

  unsigned getRank() const;
  void setRank(unsigned _rank);
  unsigned getHeight() const;
  void setHeight(unsigned _h);
  unsigned GetKeyNum() const;
  bool SetKeyNum(int _num);
  bool AddKeyNum();
  bool SubKeyNum();
  unsigned GetStore() const;
  void setStore(unsigned _store);
  unsigned getFlag() const;
  void setFlag(unsigned _flag);

  const Bstr* getKey(int _index) const;	//need to check the index
  bool SetKey(const Bstr* _key, int _index, bool ifcopy = false);
  bool addKey(const Bstr* _key, int _index, bool ifcopy = false);
  bool addKey(char* _str, unsigned _len, int _index, bool ifcopy = false);
  bool subKey(int _index, bool ifdel = false);

  //several binary key Search utilities
  int searchKey_less(const Bstr& _bstr) const;
  int searchKey_equal(const Bstr& _bstr) const;
  int searchKey_lessEqual(const Bstr& _bstr) const;

  int searchKey_less(const char* _str, unsigned _len) const;
  int searchKey_equal(const char* _str, unsigned _len) const;
  int searchKey_lessEqual(const char* _str, unsigned _len) const;

  //virtual functions: polymorphic
  //NOTICE: not pure-virtual, not required to be implemented again, can be used now
  virtual SINode* GetChild(int _index) const { return NULL; };

  /**
   * set this one's child
   * @param _child the added child
   * @param _index the position it will be placed
   * @return
   */
  virtual bool setChild(SINode* _child, int _index) { return true; };

  /**
   * add SINode into key[_index] in this node.
   * This operation will move keys after _index one position to right
   * @param _child child node
   * @param _index the inserted position
   * @return bool
   */
  virtual bool AddChild(SINode* _child, int _index) { return true; };
  virtual bool subChild(int _index) { return true; };
  virtual SINode* GetPrev() const { return NULL; };
  virtual SINode* GetNext() const { return NULL; };
  virtual unsigned GetValue(int _index) const { return -1; };
  virtual bool SetValue(unsigned _val, int _index) { return true; };
  virtual bool AddValue(unsigned _val, int _index) { return true; };
  virtual bool SubValue(int _index) { return true; };
  virtual void setPrev(SINode* _prev) {};
  virtual void SetNext(SINode* _next) {};

  /** let the node has no keys nor values in memory*/
  virtual void Virtual() = 0;

  /** Alloc keys ( values maybe) and set memory request*/
  virtual void Normal() = 0;

  /** @return the total memory size owned by this node */
  virtual unsigned GetSize() const = 0;

  /**
   * This node has too many children, so to spilt it into two nodes.
   * and add the new node into parent node
   * @param _parent pointer to parent node
   * @param _index the position of this node in parent node
   * @return
   */
  virtual SINode* Split(SINode* _parent, int _index) = 0;

  virtual SINode* Coalesce(SINode* _father, int _index) = 0;

  /**
  * release the node, only remain necessary information.
  * release all its keys ( because b_str cannot delete directly)
  * but not change its node flag information (other member variable
  * will be deleted default in Destructor function)
  */
  virtual void Release() = 0;		// virtual ~SINode() {};
  virtual void print(std::string s) = 0;		//DEBUG(print the Node)
};

/*NOTICE(operations in release())
*To Save memory, we can only remain store and node_flag_(childs added for Leaf).
*However, in this way childs'pointers is ok to change, use Node** or Node*& is also nonsense
*because the pointer variable may die.
*Another way is only to release dynamic memory, and store thw whole, read the Bstr only to
*build. In this way nodes'pointer doesn't change, and operation is simplified, while memory
*is consumed a bit more. Because Bstrs consume the most memory, and memory-disk swapping is
*the most time-consuming thing, it seems to be a better way.
*WARN:when a node is in memory and not deleted, its basic content is always being! If nodes are
*really too many, this will cause disaster because we can't swap them out until tree is closed!
*To solve this problem, there should be two types of release-function: one to release Bstr, one
*to release the whole(pointer is invalid and rebuild problem)
*/

#endif

