/*=============================================================================
# Filename: ISLeafNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:39
# Description: the leaf-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_ISTREE_NODE_ISLEAFNODE_H
#define _KVSTORE_ISTREE_NODE_ISLEAFNODE_H

#include "ISNode.h"

class ISLeafNode : public ISNode {
  protected:
  ISNode* prev; //LeafNode
  ISNode* next;
  Bstr* values;
  void AllocValues();
  //void FreeValues();
  public:
  ISLeafNode();
  ISLeafNode(bool isVirtual);
  //LeafNode(Storage* TSM);
  void Virtual();
  void Normal();
  ISNode* getPrev() const;
  ISNode* getNext() const;

  const Bstr* getValue(int _index) const;
  bool setValue(const Bstr* _value, int _index, bool ifcopy = false);
  bool addValue(const Bstr* _value, int _index, bool ifcopy = false);
  bool subValue(int _index, bool ifdel = false);
  void setPrev(ISNode* _prev);
  void setNext(ISNode* _next);
  unsigned getSize() const;

  bool setValue(char* _str, unsigned _len, int _index, bool ifcopy = false);
  bool addValue(char* _str, unsigned _len, int _index, bool ifcopy = false);

  ISNode* split(ISNode* _father, int _index);
  ISNode* coalesce(ISNode* _father, int _index);
  void release();
  ~ISLeafNode();
  void print(std::string s); //DEBUG
                             /*non-sense virtual function
	Node* getChild(int _index) const;
	bool addChild(Node* _child, int _index);
	bool subChild(int _index);
	*/
};
//BETTER: prev isn't a must, and reverse-range can be achieved using recursive-next

#endif
