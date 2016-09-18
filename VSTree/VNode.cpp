/*=============================================================================
# Filename: VNode.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 14:08
# Description: by liyouhuan and hanshuo
=============================================================================*/

#include "VNode.h"

using namespace std;

VNode::VNode()
{
    this->is_leaf = false;
    this->is_root = false;
    this->child_num = 0;
    this->self_file_line = -1;
    this->father_file_line = -1;
    for(int i = 0; i < VNode::MAX_CHILD_NUM; i ++)
    {
    	this->child_file_lines[i] = -1;
    }
}

bool 
VNode::isLeaf() const
{
    return this->is_leaf;
}

bool 
VNode::isRoot() const
{
    return this->is_root;
}

bool 
VNode::isFull() const
{
    return (this->child_num == VNode::MAX_CHILD_NUM - 1); // need one slot for splitting node.
}

void 
VNode::setAsLeaf(bool _isLeaf)
{
    this->is_leaf = _isLeaf;
}

void 
VNode::setAsRoot(bool _isRoot)
{
    this->is_root = _isRoot;
}

int 
VNode::getChildNum() const
{
    return this->child_num;
}

int 
VNode::getFileLine() const
{
    return this->self_file_line;
}

int 
VNode::getFatherFileLine() const
{
    return this->father_file_line;
}

int 
VNode::getChildFileLine(int _i) const
{
    return this->child_file_lines[_i];
}

void 
VNode::setChildNum(int _num)
{
    this->child_num = _num;
}

void 
VNode::setFileLine(int _line)
{
    this->self_file_line = _line;
}

void VNode::setFatherFileLine(int _line)
{
    this->father_file_line = _line;
}

void VNode::setChildFileLine(int _i, int _line)
{
    this->child_file_lines[_i] = _line;
}

const SigEntry& VNode::getEntry()const
{
    return this->entry;
}

const SigEntry& VNode::getChildEntry(int _i)const
{
    return this->child_entries[_i];
}

void VNode::setEntry(const SigEntry _entry)
{
    this->entry = _entry;
}

void VNode::setChildEntry(int _i, const SigEntry _entry)
{
	/* _entry should not be an ref, because if _entry is an ref,
	 * it's possible that _entry is exactly an ref of this->child_entries[i]
	 * !!!!!!!!!!!!!!!!!!!! */
	SigEntry _sig(_entry);
    this->child_entries[_i] = _sig;

    //debug
//    {
//        if (this->getFileLine() == 0 && this->getChildFileLine(_i) == 153)
//        {
//            Util::logging("set node 0's child node 153's entry:");
//            Util::logging(Signature::BitSet2str(this->child_entries[_i].getEntitySig().entityBitSet));
//        }
//    }
}

VNode* VNode::getFather(LRUCache& _nodeBuffer)const
{
    if (this->isRoot())
    {
        return NULL;
    }

    return _nodeBuffer.get(this->getFatherFileLine());
}

VNode* VNode::getChild(int _i, LRUCache& _nodeBuffer)const
{
    if (this->isLeaf())
    {
        return NULL;
    }

    return _nodeBuffer.get(this->getChildFileLine(_i));
}

/* add one child entry to this node. when splitting this node, can add a new entry to it. */
bool VNode::addChildEntry(const SigEntry _entry, bool _is_splitting)
{
    //if (this->isFull() && !_is_splitting)
    //{
        //cerr<< "error, can not add child entry when the node is full, in VNode::addChildEntry." << endl;
        //return false;
    //}

    this->setChildEntry(this->child_num, _entry);
    this->child_num ++;

    return true;
}

/* add one child node to this node. when splitting this node, can add a new child to it. */
bool VNode::addChildNode(VNode* _p_child_node, bool _is_splitting)
{
	//if (this->isFull() && !_is_splitting)
	//{
		//cerr<< "error, can not add child when the node is full. @VNode::addChildNode" << endl;
		//return false;
	//}

    //if (_p_child_node == NULL)
    //{
        //cerr<< "error, can not add child when the child node pointer is NULL. @VNode::addChildNode" << endl;
        //return false;
    //}

    _p_child_node->setFatherFileLine(this->self_file_line);
    this->setChildFileLine(this->child_num, _p_child_node->getFileLine());
    this->addChildEntry( _p_child_node->getEntry(), _is_splitting);
	//NOTICE:this function calls addChildEntry(), which already add the child_num
    //this->child_num ++;

    return true;
}

bool VNode::removeChild(int _i)
{
    if (_i < 0 || _i >= this->child_num)
    {
        cerr<< "error, illegal child index. @VNode::removeChild" << endl;
        return false;
    }

    for (int j = _i + 1; j < this->child_num; ++j)
    {
        child_entries[j-1] = child_entries[j];
        child_file_lines[j-1] = child_file_lines[j];
    }
    this->child_num --;

    return true;
}

int VNode::getIndexInFatherNode(LRUCache& _nodeBuffer)
{
    VNode * fatherNodePtr = this->getFather(_nodeBuffer);

    if (fatherNodePtr == NULL)
    {
        return 0;
    }

    int n = fatherNodePtr->getChildNum();
    for (int i = 0; i < n; i++)
    {
        if (fatherNodePtr->getChildFileLine(i) == this->self_file_line)
        {
            return i;
        }
    }

    cerr << "error, can not find rank in father node. @VNode::getRankFatherNode" << endl;
    return 0;
}

void VNode::refreshSignature()
{
    EntitySig sig;

    for (int i=0;i<this->child_num;i++)
    {
        sig |= this->child_entries[i].getEntitySig();
    }

    // refresh self node's signature.
    if (sig != this->entry.getEntitySig())
    {

        SigEntry newEntry(sig, this->entry.getEntityId());
        this->setEntry(newEntry);
    }
}

void VNode::refreshAncestorSignature(LRUCache& _nodeBuffer)
{
    // refresh self node's signature.
    this->refreshSignature();

    // refresh father node's signature.
    VNode* fatherNodePtr = this->getFather(_nodeBuffer);
    if (fatherNodePtr == NULL)
    {
        if (!this->isRoot())
            cerr << "error, can not find father node. @VNode::refreshSignature" << endl;
        return;
    }

    int rank = this->getIndexInFatherNode(_nodeBuffer);
    if (fatherNodePtr->getChildEntry(rank).getEntitySig() != this->entry.getEntitySig())
    {
        fatherNodePtr->setChildEntry(rank, this->entry);
        fatherNodePtr->refreshAncestorSignature(_nodeBuffer);
    }
}

bool VNode::retrieveChild(vector<VNode*>& _child_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer)
{
    if (this->isLeaf())
    {
        cerr << "error, can not retrieve children from a leaf node.@VNode::retrieveChild" << endl;
        return false;
    }

    for (int i=0;i<this->child_num;i++)
    {
        if (this->child_entries[i].cover(_filter_sig))
        {
            _child_vec.push_back(this->getChild(i, _nodeBuffer));
        }
    }

    return true;
}

bool VNode::retrieveEntry(vector<SigEntry>& _entry_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer)
{
    if (!this->isLeaf())
    {
        cerr << "error, can not retrieve entries from a non-leaf node. @VNode::retrieveEntry" << endl;
        return false;
    }

    for (int i=0;i<this->child_num;i++)
    {
        if (this->child_entries[i].cover(_filter_sig))
        {
            _entry_vec.push_back(this->child_entries[i]);
        }
    }

    return false;
}

bool VNode::checkState()
{
    if (this->getFileLine() < 0)
        return false;

    for (int i=0;i<this->child_num;i++)
        if (!this->isLeaf() && this->getChildFileLine(i) < 0)
        {
            return false;
        }

    return true;
}
std::string VNode::to_str()
{
	std::stringstream _ss;
	_ss << "VNode:" << endl;
	_ss << "\tEntityID:" << entry.getEntityId() << endl;
	_ss << "\tisLeaf:" << this->is_leaf << endl;
	_ss << "\tisRoot:" << this->is_root << endl;
	_ss << "\tfileline:" << this->self_file_line << endl;
	_ss << "\tsignature:" <<
		Signature::BitSet2str(this->entry.getEntitySig().entityBitSet ) << endl;
	_ss << "\tchildNum:" << this->child_num << endl << "\t";
	for(int i = 0; i < this->child_num; i ++)
	{
		if(! this->isLeaf()){
			_ss << "[" << this->getChildFileLine(i) << "]\t";
		}
		else
		{
			_ss << i << "[*" << this->getChildEntry(i).getEntityId() << "]\t";
			_ss << (this->getChildEntry(i).to_str()) << endl;
		}
		_ss << "\t" << this->child_entries[i].to_str() << endl;
	}
	_ss << endl << endl;

	return _ss.str();
}
