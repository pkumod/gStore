/*=============================================================================
# Filename: ISIntlNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: the internal-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_ISTREE_NODE_ISINTLNODE_H
#define _KVSTORE_ISTREE_NODE_ISINTLNODE_H

#include "ISNode.h"

class ISIntlNode : public ISNode {
  protected:
  ISNode* childs[MAX_CHILD_NUM + 1];
  //Node** childs;
  //void AllocChilds();
  public:
  ISIntlNode();
  ISIntlNode(bool isVirtual);
  //IntlNode(Storage* TSM);
  void Virtual();
  void Normal();
  ISNode* getChild(int _index) const;
  bool setChild(ISNode* _child, int _index);
  bool addChild(ISNode* _child, int _index);
  bool subChild(int _index);
  unsigned getSize() const;
  ISNode* split(ISNode* _father, int _index);
  ISNode* coalesce(ISNode* _father, int _index);
  void release();
  ~ISIntlNode();
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
