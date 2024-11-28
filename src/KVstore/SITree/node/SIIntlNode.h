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

class SIIntlNode : public SINode
{
protected:
	std::shared_ptr<SINode> childs[MAX_CHILD_NUM + 1];
public:
	SIIntlNode();
	SIIntlNode(bool isVirtual);
	//IntlNode(Storage* tsm_);
	void Virtual();
	void Normal();
	std::shared_ptr<SINode> GetChild(int _index) const;
	bool setChild(std::shared_ptr<SINode> _child, int _index);
	bool AddChild(std::shared_ptr<SINode> _child, int _index);
	bool subChild(int _index);
	unsigned GetSize() const;
	std::shared_ptr<SINode> Split(std::shared_ptr<SINode> _parent, int _index);
	std::shared_ptr<SINode> Coalesce(std::shared_ptr<SINode> _father, int _index);
	void Release();
	~SIIntlNode();
	void print(std::string s);	//DEBUG
	/*non-sense functions: polymorphic
	Node* GetPrev() const;
	Node* GetNext() const;
	const Bstr* GetValue(int _index) const;
	bool SetValue(const Bstr* _value, int _index);
	bool AddValue(const Bstr* _value, int _index);
	bool SubValue(int _index);
	void setPrev(Node* _prev);
	void SetNext(Node* _next);
	*/
};

#endif

