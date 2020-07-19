/*=============================================================================
# Filename: SIIntlNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: the internal-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_SITREE_NODE_SIINTLNODE_H
#define _KVSTORE_SITREE_NODE_SIINTLNODE_H

#include "SINode.h"

class SIIntlNode : public SINode {
  protected:
  SINode* childs[MAX_CHILD_NUM + 1];
  //Node** childs;
  //void AllocChilds();
  public:
  SIIntlNode();
  SIIntlNode(bool isVirtual);
  //IntlNode(Storage* TSM);
  void Virtual();
  void Normal();
  SINode* getChild(int _index) const;
  bool setChild(SINode* _child, int _index);
  bool addChild(SINode* _child, int _index);
  bool subChild(int _index);
  unsigned getSize() const;
  SINode* split(SINode* _father, int _index);
  SINode* coalesce(SINode* _father, int _index);
  void release();
  ~SIIntlNode();
  void print(std::string s); //DEBUG
                             /*non-sense functions: polymorphic
	Node* getPrev() const;
	Node* getNext() const;
	const Bstr* getValue(int _index) const;
	bool setValue(const Bstr* _value, int _index);
	bool addValue(const Bstr* _value, int _index);
	bool subValue(int _index);
	void setPrev(Node* _prev);
	void setNext(Node* _next);
	*/
};

#endif
