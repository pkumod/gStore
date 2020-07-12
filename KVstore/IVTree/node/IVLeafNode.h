/*=============================================================================
# Filename: IVLeafNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: the leaf-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_IVTREE_NODE_IVLEAFNODE_H
#define _KVSTORE_IVTREE_NODE_IVLEAFNODE_H

#include "IVNode.h"

class IVLeafNode : public IVNode {
  protected:
  IVNode* prev; //LeafNode
  IVNode* next;
  Bstr* values;
  void AllocValues();
  //void FreeValues();
  public:
  IVLeafNode();
  IVLeafNode(bool isVirtual);
  //LeafNode(Storage* TSM);
  void Virtual();
  void Normal();
  IVNode* getPrev() const;
  IVNode* getNext() const;
  const Bstr* getValue(int _index) const;
  bool setValue(const Bstr* _value, int _index, bool _ifcopy = false);

  bool getValue(VList* _vlist, int _index, char*& _str, unsigned& _len) const;
  bool setValue(VList* _vlist, int _index, char* _str, unsigned _len, bool ifcopy = false);

  bool addValue(VList* _vlist, int _index, char* _str, unsigned _len, bool ifcopy = false);
  bool subValue(VList* _vlist, int _index, bool ifdel = false);
  bool addValue(const Bstr* _val, int _index, bool ifcopy = false);
  bool subValue(int _index, bool ifdel = false);

  void setPrev(IVNode* _prev);
  void setNext(IVNode* _next);
  unsigned getSize() const;
  IVNode* split(IVNode* _father, int _index);
  IVNode* coalesce(IVNode* _father, int _index);
  void release();
  ~IVLeafNode();
  void print(std::string s); //DEBUG
                             /*non-sense virtual function
	Node* getChild(int _index) const;
	bool addChild(Node* _child, int _index);
	bool subChild(int _index);
	*/
};
//BETTER: prev isn't a must, and reverse-range can be achieved using recursive-next

#endif
