/*=============================================================================
# Filename: SILeafNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: the leaf-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_SITREE_NODE_SILEAFNODE_H
#define _KVSTORE_SITREE_NODE_SILEAFNODE_H

#include "SINode.h"

/**
 * The leaf node of SITree. It maintains linked list structures.
 * And a list of values, which is ID corresponding to strings in keys.
 */
class SILeafNode : public SINode
{
 protected:
  SINode* prev;	//LeafNode
  SINode* next;
  unsigned* values;
  void AllocValues();
 public:
  SILeafNode();
  SILeafNode(bool isVirtual);
  void Virtual();
  void Normal();
  SINode* GetPrev() const;
  SINode* GetNext() const;
  unsigned GetValue(int _index) const;
  bool SetValue(unsigned _val, int _index);
  bool AddValue(unsigned _val, int _index);

  bool SubValue(int _index);

  void setPrev(SINode* _prev);
  void SetNext(SINode* _next);

  unsigned GetSize() const;

  SINode* Split(SINode* _parent, int _index);
  SINode* Coalesce(SINode* _parent, int _index);

  void Release();
  ~SILeafNode();
  void print(std::string s);			//DEBUG

  /*non-sense virtual function
  Node* GetChild(int _index) const;
  bool AddChild(Node* _child, int _index);
  bool subChild(int _index);
  */
};

#endif

