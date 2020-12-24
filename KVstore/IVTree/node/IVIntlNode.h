/*=============================================================================
# Filename: IVIntlNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: the internal-node of a B+ tree
=============================================================================*/

#ifndef _KVSTORE_IVTREE_NODE_IVINTLNODE_H
#define _KVSTORE_IVTREE_NODE_IVINTLNODE_H

#include "IVNode.h"

class IVIntlNode : public IVNode {
  protected:
  IVNode* childs[MAX_CHILD_NUM + 1];
  //Node** childs;
  //void AllocChilds();
  public:
  IVIntlNode();
  IVIntlNode(bool isVirtual);
  //IntlNode(Storage* TSM);
  void Virtual();
  void Normal();
  IVNode* getChild(int _index) const;
  bool setChild(IVNode* _child, int _index);
  bool addChild(IVNode* _child, int _index);
  bool subChild(int _index);
  unsigned getSize() const;
  IVNode* split(IVNode* _father, int _index);
  IVNode* coalesce(IVNode* _father, int _index);
  void release();
  ~IVIntlNode();
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
