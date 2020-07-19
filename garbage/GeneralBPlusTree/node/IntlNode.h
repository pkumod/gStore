/*=============================================================================
# Filename: IntlNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: the internal-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_NODE_INTLNODE_H
#define _KVSTORE_NODE_INTLNODE_H

#include "Node.h"

class IntlNode : public Node {
  protected:
  Node* childs[MAX_CHILD_NUM + 1];
  //Node** childs;
  //void AllocChilds();
  public:
  IntlNode();
  IntlNode(bool isVirtual);
  //IntlNode(Storage* TSM);
  void Virtual();
  void Normal();
  Node* getChild(int _index) const;
  bool setChild(Node* _child, int _index);
  bool addChild(Node* _child, int _index);
  bool subChild(int _index);
  unsigned getSize() const;
  Node* split(Node* _father, int _index);
  Node* coalesce(Node* _father, int _index);
  void release();
  ~IntlNode();
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
