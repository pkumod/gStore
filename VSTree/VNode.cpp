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
  //this->is_leaf = false;
  //this->is_root = false;
  //this->child_num = 0;
  this->flag = 0;
  this->self_file_line = -1;
  this->father_file_line = -1;

  this->child_file_lines = new int[VNode::MAX_CHILD_NUM];
  for (int i = 0; i < VNode::MAX_CHILD_NUM; i++) {
    this->child_file_lines[i] = -1;
  }

  InitLock();
}

VNode::VNode(bool _is_leaf)
{
  //this->is_leaf = false;
  //this->is_root = false;
  //this->child_num = 0;
  this->flag = 0;
  this->self_file_line = -1;
  this->father_file_line = -1;

  if (_is_leaf) {
    this->child_file_lines = NULL;
    //return;
  } else {
    this->AllocChilds();
  }

  InitLock();
}

VNode::~VNode()
{
  delete[] this->child_file_lines;
  this->child_file_lines = NULL;

#ifdef THREAD_VSTREE_ON
  pthread_mutex_destroy(&(this->node_lock));
#endif
}

void
VNode::AllocChilds()
{
  this->child_file_lines = new int[VNode::MAX_CHILD_NUM];
  for (int i = 0; i < VNode::MAX_CHILD_NUM; i++) {
    this->child_file_lines[i] = -1;
  }
}

void
VNode::InitLock()
{
#ifdef THREAD_VSTREE_ON
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&(this->node_lock), &attr);
#endif
}

void
VNode::setFlag(unsigned _flag)
{
  this->flag = _flag;
}

bool
VNode::isDirty() const
{
  return this->flag & VNode::DIRTY_PART;
}

void
VNode::setDirty(bool _flag)
{
  if (_flag) {
    this->flag |= VNode::DIRTY_PART;
  } else {
    this->flag &= VNode::DEL_DIRTY_PART;
  }
}

bool
VNode::isLeaf() const
{
  return this->flag & VNode::LEAF_PART;
}

bool
VNode::isRoot() const
{
  return this->flag & VNode::ROOT_PART;
}

bool
VNode::isFull() const
{
  return (this->getChildNum() == VNode::MAX_CHILD_NUM - 1); // need one slot for splitting node.
}

void
VNode::setAsLeaf(bool _isLeaf)
{
  if (_isLeaf) {
    this->flag |= VNode::LEAF_PART;
  } else {
    this->flag &= VNode::DEL_LEAF_PART;
  }
}

void
VNode::setAsRoot(bool _isRoot)
{
  if (_isRoot) {
    this->flag |= VNode::ROOT_PART;
  } else {
    this->flag &= VNode::DEL_ROOT_PART;
  }
  //this->setDirty();
}

int
VNode::getChildNum() const
{
  //return this->child_num;
  return this->flag & VNode::NUM_PART;
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
  //this->child_num = _num;
  this->flag &= VNode::DEL_NUM_PART;
  this->flag |= _num;
  //this->setDirty();
}

void
VNode::setFileLine(int _line)
{
  this->self_file_line = _line;
  //this->setDirty();
}

void VNode::setFatherFileLine(int _line)
{
  this->father_file_line = _line;
}

void VNode::setChildFileLine(int _i, int _line)
{
  this->child_file_lines[_i] = _line;
}

const SigEntry& VNode::getEntry() const
{
  return this->entry;
}

const SigEntry& VNode::getChildEntry(int _i) const
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

VNode* VNode::getFather(LRUCache& _nodeBuffer) const
{
  if (this->isRoot()) {
    return NULL;
  }

  return _nodeBuffer.get(this->getFatherFileLine());
}

VNode* VNode::getChild(int _i, LRUCache& _nodeBuffer) const
{
  if (this->isLeaf()) {
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

  int child_num = this->getChildNum();
  this->setChildEntry(child_num, _entry);
  //this->child_num ++;
  this->setChildNum(child_num + 1);

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

  int child_num = this->getChildNum();
  _p_child_node->setFatherFileLine(this->self_file_line);
  _p_child_node->setDirty();
  this->setChildFileLine(child_num, _p_child_node->getFileLine());
  this->addChildEntry(_p_child_node->getEntry(), _is_splitting);
  //NOTICE:this function calls addChildEntry(), which already add the child_num
  //this->child_num ++;

  return true;
}

bool VNode::removeChild(int _i)
{
  int child_num = this->getChildNum();
  if (_i < 0 || _i >= child_num) {
    cerr << "error, illegal child index. @VNode::removeChild" << endl;
    return false;
  }

  if (this->isLeaf()) {
    for (int j = _i + 1; j < child_num; ++j) {
      child_entries[j - 1] = child_entries[j];
      //child_file_lines[j-1] = child_file_lines[j];
    }
  } else {
    for (int j = _i + 1; j < child_num; ++j) {
      child_entries[j - 1] = child_entries[j];
      child_file_lines[j - 1] = child_file_lines[j];
    }
  }

  //this->child_num --;
  this->setChildNum(child_num - 1);

  return true;
}

int VNode::getIndexInFatherNode(LRUCache& _nodeBuffer)
{
  VNode* fatherNodePtr = this->getFather(_nodeBuffer);

  if (fatherNodePtr == NULL) {
    return 0;
  }

  int n = fatherNodePtr->getChildNum();
  for (int i = 0; i < n; i++) {
    if (fatherNodePtr->getChildFileLine(i) == this->self_file_line) {
#ifdef THREAD_VSTREE_ON
      pthread_mutex_unlock(&(fatherNodePtr->node_lock));
#endif
      return i;
    }
  }

#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(fatherNodePtr->node_lock));
#endif
  cerr << "error, can not find rank in father node. @VNode::getIndexInFatherNode" << endl;
  return 0;
}

void VNode::refreshSignature()
{
  EntitySig sig;
  int child_num = this->getChildNum();

  for (int i = 0; i < child_num; i++) {
    sig |= this->child_entries[i].getEntitySig();
  }

  // refresh self node's signature.
  if (sig != this->entry.getEntitySig()) {

    SigEntry newEntry(sig, this->entry.getEntityId());
    this->setEntry(newEntry);
  }
}

void VNode::refreshAncestorSignature(LRUCache& _nodeBuffer)
{
  // refresh self node's signature.
  this->refreshSignature();

// refresh father node's signature.
#ifdef DEBUG_VSTREE
//cout<<"VNode::refreshAncestorSignature() - to get father"<<endl;
#endif
  VNode* fatherNodePtr = this->getFather(_nodeBuffer);
  if (fatherNodePtr == NULL) {
    if (!this->isRoot())
      cerr << "error, can not find father node. @VNode::refreshSignature" << endl;
    return;
  }

  int rank = this->getIndexInFatherNode(_nodeBuffer);
  if (fatherNodePtr->getChildEntry(rank).getEntitySig() != this->entry.getEntitySig()) {
    fatherNodePtr->setDirty();
    fatherNodePtr->setChildEntry(rank, this->entry);
    fatherNodePtr->refreshAncestorSignature(_nodeBuffer);
  }
#ifdef THREAD_VSTREE_ON
  pthread_mutex_unlock(&(fatherNodePtr->node_lock));
#endif
}

bool VNode::retrieveChild(vector<VNode*>& _child_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer)
{
  if (this->isLeaf()) {
    cerr << "error, can not retrieve children from a leaf node.@VNode::retrieveChild" << endl;
    return false;
  }

  int child_num = this->getChildNum();
  for (int i = 0; i < child_num; i++) {
    if (this->child_entries[i].cover(_filter_sig)) {
      _child_vec.push_back(this->getChild(i, _nodeBuffer));
    }
  }

  return true;
}

bool VNode::retrieveEntry(vector<SigEntry>& _entry_vec, const EntitySig _filter_sig, LRUCache& _nodeBuffer)
{
  if (!this->isLeaf()) {
    cerr << "error, can not retrieve entries from a non-leaf node. @VNode::retrieveEntry" << endl;
    return false;
  }

  int child_num = this->getChildNum();
  for (int i = 0; i < child_num; i++) {
    if (this->child_entries[i].cover(_filter_sig)) {
      _entry_vec.push_back(this->child_entries[i]);
    }
  }

  return false;
}

bool VNode::checkState()
{
  if (this->getFileLine() < 0)
    return false;

  int child_num = this->getChildNum();
  for (int i = 0; i < child_num; i++)
    if (!this->isLeaf() && this->getChildFileLine(i) < 0) {
      return false;
    }

  return true;
}
std::string VNode::to_str()
{
  std::stringstream _ss;
  _ss << "VNode:" << endl;
  _ss << "\tEntityID:" << entry.getEntityId() << endl;
  _ss << "\tisLeaf:" << this->isLeaf() << endl;
  _ss << "\tisRoot:" << this->isRoot() << endl;
  _ss << "\tfileline:" << this->self_file_line << endl;
  _ss << "\tsignature:" << Signature::BitSet2str(this->entry.getEntitySig().entityBitSet) << endl;
  int child_num = this->getChildNum();
  _ss << "\tchildNum:" << child_num << endl << "\t";
  for (int i = 0; i < child_num; i++) {
    if (!this->isLeaf()) {
      _ss << "[" << this->getChildFileLine(i) << "]\t";
    } else {
      _ss << i << "[*" << this->getChildEntry(i).getEntityId() << "]\t";
      _ss << (this->getChildEntry(i).to_str()) << endl;
    }
    _ss << "\t" << this->child_entries[i].to_str() << endl;
  }
  _ss << endl << endl;

  return _ss.str();
}

//TODO: keep a lock for each node, but not write to disk

bool
VNode::readNode(FILE* _fp)
{
  int ret = fread(&(this->flag), sizeof(unsigned), 1, _fp);
  if (ret == 0) //the edn of file
  {
    return false;
  }
  fread(&(this->self_file_line), sizeof(int), 1, _fp);
  //cout<<"to read node: "<<this->self_file_line<<endl;
  //BETTER:if the id < 0, then just move, not read

  fread(&(this->father_file_line), sizeof(int), 1, _fp);
  fread(&(this->entry), sizeof(SigEntry), 1, _fp);

  //for(int i = 0; i < VNode::MAX_CHILD_NUM; ++i)
  //{
  //fread(&(this->child_entries[i]), sizeof(SigEntry), 1, _fp);
  //}
  fread(this->child_entries, sizeof(SigEntry), VNode::MAX_CHILD_NUM, _fp);

  if (!this->isLeaf()) //internal node
  {
    this->child_file_lines = new int[VNode::MAX_CHILD_NUM];
    //for(int i = 0; i < VNode::MAX_CHILD_NUM; ++i)
    //{
    //fread(&(this->child_file_lines[i]), sizeof(int), 1, _fp);
    //}
    fread(this->child_file_lines, sizeof(int), VNode::MAX_CHILD_NUM, _fp);
  } else //move to the end of the node block
  {
    fseek(_fp, sizeof(int) * VNode::MAX_CHILD_NUM, SEEK_CUR);
  }
  //this->setDirty(false);

  return true;
}

bool
VNode::writeNode(FILE* _fp)
{
  //clean, then no need to write
  //if(!this->isDirty())
  //{
  //return true;
  //}
  //NOTICE:already dealed in LRUCache
  //this->setDirty(false);

  //cout<<"to write node: "<<this->self_file_line<<endl;
  fwrite(&(this->flag), sizeof(unsigned), 1, _fp);
  fwrite(&(this->self_file_line), sizeof(int), 1, _fp);
  //NOTICE: this must be a old node(not new inserted node), so no need to write more
  if (this->self_file_line < 0) {
    return true;
  }

  fwrite(&(this->father_file_line), sizeof(int), 1, _fp);
  fwrite(&(this->entry), sizeof(SigEntry), 1, _fp);

  //for(int i = 0; i < VNode::MAX_CHILD_NUM; ++i)
  //{
  //fwrite(&(this->child_entries[i]), sizeof(SigEntry), 1, _fp);
  //}
  fwrite(this->child_entries, sizeof(SigEntry), VNode::MAX_CHILD_NUM, _fp);

  if (!this->isLeaf()) //internal node
  {
    //for(int i = 0; i < VNode::MAX_CHILD_NUM; ++i)
    //{
    //fwrite(&(this->child_file_lines[i]), sizeof(int), 1, _fp);
    //}
    fwrite(this->child_file_lines, sizeof(int), VNode::MAX_CHILD_NUM, _fp);
  } else //move to the end of the node block
  {
    //fseek(_fp, sizeof(int) * VNode::MAX_CHILD_NUM, SEEK_CUR);
    int t = 0;
    for (int i = 0; i < VNode::MAX_CHILD_NUM; ++i) {
      fwrite(&t, sizeof(int), 1, _fp);
    }
  }

  return true;
}
