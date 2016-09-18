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

class VNode
{
public:
	static const int DEGREE = 100;  //use 100 for normal
	//NOTICE+WARN:always ensure double times due to union operation safety in coalesce
	//here 201 is used to ensure split operation is right
	//(generally, 200 is ok if dealed carefully, but here only 200 is really used)
    static const int MAX_CHILD_NUM = 2 * VNode::DEGREE + 1;
    //static const int MAX_CHILD_NUM = 151;
    static const int MIN_CHILD_NUM = VNode::DEGREE;
    //static const int MIN_CHILD_NUM = 60;

    //debug
//    static const int MAX_CHILD_NUM = 50;
//    static const int MIN_CHILD_NUM = 20;
	VNode();
	bool isLeaf()const;
	bool isRoot()const;
	bool isFull()const;
	void setAsLeaf(bool _isLeaf);
	void setAsRoot(bool _isRoot);
	int getChildNum()const;
	int getFileLine()const;
	int getFatherFileLine()const;
	int getChildFileLine(int _i)const;
	void setChildNum(int _num);
    void setFileLine(int _line);
    void setFatherFileLine(int _line);
    void setChildFileLine(int _i, int _line);
	const SigEntry& getEntry()const;
	const SigEntry& getChildEntry(int _i)const;
    void setEntry(const SigEntry _entry);
    void setChildEntry(int _i, const SigEntry _entry);
	VNode* getFather(LRUCache& _nodeBuffer)const; // get the father node's pointer.
	VNode* getChild(int _i, LRUCache& _nodeBuffer)const; // get the _i-th child node's pointer.
	/* add one child node to this node. when splitting this node, can add a new child to it. */
	bool addChildNode(VNode* _p_child_node, bool _is_splitting = false);
	/* add one child entry to this node. when splitting this node, can add a new entry to it. */
	bool addChildEntry(const SigEntry _entry, bool _is_splitting = false);
	bool removeChild(int _i);
	int getIndexInFatherNode(LRUCache& _nodeBuffer);
	void refreshSignature(); // just refresh itself signature.
	void refreshAncestorSignature(LRUCache& _nodeBuffer); // refresh self and its ancestor's signature.
	/* used by internal Node */
	bool retrieveChild(std::vector<VNode*>& _child_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer);
	/* only used by leaf Node */
	bool retrieveEntry(std::vector<SigEntry>& _entry_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer);

	 //for debug 
	bool checkState();

	std::string to_str();

private:
    bool is_leaf;
    bool is_root;
    int child_num;
    int self_file_line;
    int father_file_line;
    SigEntry entry;
	//BETTER:is this necessary? too much memory?
	//DEBUG:add 1 in case of error
    SigEntry child_entries[VNode::MAX_CHILD_NUM];
    int child_file_lines[VNode::MAX_CHILD_NUM];
};

#endif // _VSTREE_VNODE_H

