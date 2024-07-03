/*=============================================================================
# Filename: SITree.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:45
# Description: achieve functions in SITree.h
=============================================================================*/

#include "SITree.h"

using namespace std;

SITree::SITree()
{
  height_ = 0;
  mode_ = "";
  root_ = NULL;
  leaves_head_ = NULL;
  leaves_tail_ = NULL;
  tsm_ = NULL;
  store_path_ = "";
  filename_ = "";
  this->request_ = 0;
  this->if_single_thread = false;
}

/**
 * build an SITree, if the database has been built, the mode should be "open"
 * @param store_path file path
 * @param _filename file name
 * @param _mode "open"/"build"
 * @param _buffer_size the required buffer size
 */
SITree::SITree(string store_path, string _filename, string _mode, unsigned long long _buffer_size)
{
  this->root_ = nullptr;
  this->leaves_head_ = nullptr;
  this->leaves_tail_ = nullptr;
  store_path_ = store_path;
  filename_ = _filename;
  this->height_ = 0;
  this->mode_ = string(_mode);
  string filepath = this->GetFilePath();
  tsm_ = new SIStorage(filepath, this->mode_, &this->height_, _buffer_size);
  if (this->mode_ == "open") {
    // load the entire SITree
    this->tsm_->PreRead(this->root_, this->leaves_head_, this->leaves_tail_);
  }

  this->request_ = 0;
  this->if_single_thread = false;
}

string
SITree::GetFilePath()
{
  return store_path_ + "/" + filename_;
}

unsigned
SITree::GetHeight() const
{
  return this->height_;
}

void
SITree::SetHeight(unsigned _h)
{
  this->height_ = _h;
}

SINode*
SITree::GetRoot() const
{
  return this->root_;
}

void
SITree::Prepare(SINode* _np)
{
  bool flag = _np->inMem();
  if (!flag)
    this->tsm_->ReadNode(_np, &request_);	//ReadNode deal with request
}

/**
 * Search for string. And return by _val if found.
 * First find SINode and then check if string exists
 * in SINode's keys
 * @param _str string pointer
 * @param _len string length
 * @param _val pointer to string ID, modified if found
 * @return bool
 */
bool
SITree::Search(const char* _str, unsigned _len, unsigned* _val)
{
  this->access_lock_.lock();
  if (_str == NULL || _len == 0)
  {
    printf("error in SITree-Search: empty string\n");
    this->access_lock_.unlock();
    return false;
  }

  request_ = 0;
  int store;
  SINode* ret = this->Find(_str, _len, &store, false);
  if (ret == NULL || store == -1)	//tree is empty or not found
  {
    this->access_lock_.unlock();
    return false;
  }
  const Bstr* tmp = ret->getKey(store);
  if (Util::compare(_str, _len, tmp->getStr(), tmp->getLen()) != 0)	//tree is empty or not found
  {
    this->access_lock_.unlock();
    return false;
  }
  *_val = ret->GetValue(store);
  this->tsm_->request(request_);

  this->access_lock_.unlock();
  return true;
}

/**
 * Insert a string into SITree. Add the SITree memory request size if inserted.
 * And split node if the node size exceeds limit.
 * @warning Only if str is not in SITree can you insert it!
 * @param str pointer to string
 * @param len string length
 * @param val string id
 * @return bool
 */
bool
SITree::Insert(char* str, unsigned len, unsigned val)
{
  this->access_lock_.lock();
  if (str == NULL || len == 0)
  {
    printf("error in SITree-insert: empty string\n");
    this->access_lock_.unlock();
    return false;
  }

  this->request_ = 0;
  SINode* ret;
  if (this->root_ == NULL)	//tree is empty
  {
    leaves_tail_ = leaves_head_ = root_ = new SILeafNode;
    request_ += SINode::LEAF_SIZE;
    this->height_ = 1;
    root_->setHeight(1);	//add to heap later
    this->tsm_->updateHeap(root_, root_->getRank(), false);
  }

  //  split the root SINode
  if (root_->GetKeyNum() == SINode::MAX_KEY_NUM)
  {
    SINode* father = new SIIntlNode;
    request_ += SINode::INTL_SIZE;
    father->AddChild(root_, 0);
    ret = root_->Split(father, 0);
    if (ret->isLeaf() && ret->GetNext() == NULL)
      this->leaves_tail_ = ret;
    if (ret->isLeaf())
      request_ += SINode::LEAF_SIZE;
    else
      request_ += SINode::INTL_SIZE;
    this->height_++;		//height rises only when root splits
    //WARN: height area in SINode: 4 bit!
    father->setHeight(this->height_);	//add to heap later
    this->tsm_->updateHeap(ret, ret->getRank(), false);
    this->root_ = father;
    if (root_->heapId == -1)
      root_->heapId = 0;
  }

  SINode* p = this->root_;
  SINode* q;
  int i = 0;
  while (!p->isLeaf())
  {
    i = p->searchKey_less(str, len);
    q = p->GetChild(i);
    this->Prepare(q);
    if (q->GetKeyNum() == SINode::MAX_KEY_NUM)
    {
      ret = q->Split(p, i);
      if (ret->isLeaf() && ret->GetNext() == NULL)
        this->leaves_tail_ = ret;
      if (ret->isLeaf())
        request_ += SINode::LEAF_SIZE;
      else
        request_ += SINode::INTL_SIZE;
      //BETTER: in loop may update multiple times
      this->tsm_->updateHeap(ret, ret->getRank(), false);
      this->tsm_->updateHeap(q, q->getRank(), true);
      this->tsm_->updateHeap(p, p->getRank(), true);
      const Bstr* tmp = p->getKey(i);
      int cmp_res = Util::compare(str, len, tmp->getStr(), tmp->getLen());
      if (cmp_res < 0)
        p = q;
      else
        p = ret;
    }
    else
    {
      p->setDirty();
      this->tsm_->updateHeap(p, p->getRank(), true);
      p = q;
    }
  }
  i = p->searchKey_less(str, len);

  //insert existing key is ok, but not inserted in
  //however, the tree-shape may change due to possible split in former code
  bool ifexist = false;

  if (i > 0)
  {
    const Bstr* tmp = p->getKey(i-1);
    int cmp_res = Util::compare(str, len, tmp->getStr(), tmp->getLen());
    if(cmp_res == 0)
    {
      ifexist = true;
    }
  }

  if(!ifexist)
  {
    p->addKey(str, len, i, true);
    p->AddValue(val, i);
    p->AddKeyNum();
    request_ += len;
    p->setDirty();
    this->tsm_->updateHeap(p, p->getRank(), true);
  }
  else
  {
    SLOG_ERROR("error SITree::Insert exist:");
  }

  this->tsm_->request(request_);
  this->access_lock_.unlock();
  return !ifexist;		//QUERY(which case:return false)
}

/**
 * Modify a string's corresponding ID in SITree
 * @param _str string pointer
 * @param _len string length
 * @param _val the New ID
 */
bool
SITree::Modify(const char* _str, unsigned _len, unsigned _val)
{
  this->access_lock_.lock();
  if (_str == NULL || _len == 0)
  {
    printf("error in SITree-modify: empty string\n");
    this->access_lock_.unlock();
    return false;
  }
  this->request_ = 0;
  int store;
  SINode* ret = this->Find(_str, _len, &store, true);
  if (ret == NULL || store == -1)	//tree is empty or not found
  {
    this->access_lock_.unlock();
    return false;
  }
  const Bstr* tmp = ret->getKey(store);
  if (Util::compare(_str, _len, tmp->getStr(), tmp->getLen()) != 0)	//tree is empty or not found
  {
    this->access_lock_.unlock();
    return false;
  }

  ret->SetValue(_val, store);
  ret->setDirty();
  this->tsm_->request(request_);
  this->access_lock_.unlock();
  return true;
}

/**
 * this function is useful for Search and modify, and range-query
 * @param _str pointer to string
 * @param _len string length
 * @param _store  the child position(not equals to key position)
 * @param if_modify the way after Find function completed, if return node maybe
 * changed, set if_modify = true
 * @return the leaf node
 */
SINode*
SITree::Find(const char* _str, unsigned _len, int* _store, bool if_modify)
{											//to assign value for this->bstr, function shouldn't be const!
  if (this->root_ == NULL)
    return NULL;						//SITree Is Empty

  SINode* p = root_;
  int i, j;//local Bstr: multiple delete

  while (!p->isLeaf())
  {
    if (if_modify)
      p->setDirty();
    i = p->searchKey_less(_str, _len);
    p = p->GetChild(i);
    this->Prepare(p);
  }

  j = p->GetKeyNum();

  i = p->searchKey_lessEqual(_str, _len);

  if (i == j)
    *_store = -1;	//Not Found
  else
    *_store = i;

  //bstr.clear();

  return p;
}

/**
 * Delete from SITree and Coalesce node if met
 * @param _str string pointer
 * @param _len string length
 */
bool
SITree::Remove(const char* _str, unsigned _len)
{
  this->access_lock_.lock();
  if (_str == NULL || _len == 0)
  {
    printf("error in SITree-remove: empty string\n");
    this->access_lock_.unlock();
    return false;
  }
  request_ = 0;
  SINode* ret;
  if (this->root_ == NULL)	//tree is empty
  {
    this->access_lock_.unlock();
    return false;
  }
  SINode* p = this->root_;
  SINode* q;
  int i, j;
  while (!p->isLeaf())
  {
    j = p->GetKeyNum();
    i = p->searchKey_less(_str, _len);

    q = p->GetChild(i);
    this->Prepare(q);
    // if child has too few keys merge it
    if (q->GetKeyNum() < SINode::MIN_CHILD_NUM)
    {
      // prepare  left and right siblings
      if (i > 0)
        this->Prepare(p->GetChild(i - 1));
      if (i < j)
        this->Prepare(p->GetChild(i + 1));
      if (!dynamic_cast<SILeafNode *>(q) && !dynamic_cast<SIIntlNode *>(q))
      {
        SLOG_ERROR("error remove str:" << _str << " key num:" << j << " query node pos:" << i);
        break;
      }
      ret = q->Coalesce(p, i);
      if (ret != NULL)
      {
        this->tsm_->updateHeap(ret, 0, true);//non-sense node
        ret->setPrev(nullptr);
        ret->SetNext(nullptr);
      }
      this->tsm_->updateHeap(q, q->getRank(), true);

      if (q->isLeaf())
      {
        if (q->GetPrev() == nullptr)
          this->leaves_head_ = q;
        if (q->GetNext() == nullptr)
          this->leaves_tail_ = q;
      }

      if (p->GetKeyNum() == 0)		//root shrinks
      {
        this->root_ = q;
        this->tsm_->updateHeap(p, 0, true);	//instead of delete p
        p->setPrev(nullptr);
        p->SetNext(nullptr);
        this->height_--;
      }
    }
    else
      p->setDirty();
    this->tsm_->updateHeap(p, p->getRank(), true);
    p = q;
  }

  bool flag = false;
  i = p->searchKey_equal(_str, _len);
  //WARN+NOTICE:here must check, because the key to remove maybe not exist
  if (i != (int) p->GetKeyNum())
  {
    request_ -= p->getKey(i)->getLen();
    p->subKey(i, true);		//to release
    p->SubValue(i);	//to release
    p->SubKeyNum();
    p->setDirty();
    if (p->GetKeyNum() == 0)	//root leaf 0 key
    {
      this->root_ = NULL;
      this->leaves_head_ = NULL;
      this->leaves_tail_ = NULL;
      this->height_ = 0;
      this->tsm_->updateHeap(p, 0, true);	//instead of delete p
      p->setPrev(nullptr);
      p->SetNext(nullptr);
    }
    flag = true;
  }
  if (!flag)
    SLOG_ERROR("error remove data not found key:");

  this->tsm_->request(request_);
  this->access_lock_.unlock();
  return flag;		//i == j, not found
}

/**
 * Save the whole tree to disk
 * @return bool
 */
bool
SITree::Save()
{
  this->access_lock_.lock();
#ifdef DEBUG_KVSTORE
  printf("now to Save tree!\n");
#endif
  if (tsm_->WriteTree(this->root_))
  {
    this->access_lock_.unlock();
    return true;
  }
  else
  {
    this->access_lock_.unlock();
    return false;
  }
}

/**
 * Release subtree (rooted in _np ) from memory
 * @param _np
 */
void
SITree::Release(SINode* _np) const
{
  if (_np == NULL)	return;
  if (_np->isLeaf())
  {
    delete _np;
    _np = nullptr;
    return;
  }
  int cnt = _np->GetKeyNum();
  for (; cnt >= 0; --cnt)
    Release(_np->GetChild(cnt));
  delete _np;
  _np = nullptr;
}

SITree::~SITree()
{
  delete tsm_;
  tsm_ = nullptr;
#ifdef DEBUG_KVSTORE
  //printf("already empty the buffer, now to delete all nodes in tree!\n");
#endif
  //recursively delete each SINode
  Release(root_);
  root_ = nullptr;
  //cout << "~SITree done" << endl;
}

void SITree::PrintTree(SINode* _np)
{
  //foreach all keys
  if (_np == NULL)	return;

  if (_np->isLeaf())
  {
    unsigned num = _np->GetKeyNum();
    for (unsigned i = 0; i < num; ++i)
    {
      SLOG_CODE("debug PrintTree leaf:" << "   len:" << _np->getKey(i)->getLen());
    }
    return;
  }

  int cnt = _np->GetKeyNum();
  for (; cnt >= 0; --cnt)
    PrintTree(_np->GetChild(cnt));
  
  unsigned num = _np->GetKeyNum();
	for (unsigned i = 0; i < num; ++i)
    SLOG_CODE("debug PrintTree Int:" << "   len:" << _np->getKey(i)->getLen());

  // SINode* np = nullptr;
  // for (np = this->leaves_head_; np != NULL; np = np->GetNext())
  // {
  //   unsigned num = np->GetKeyNum();
  //   for (unsigned i = 0; i < num; ++i)
  //   {
  //     std::cout << "debug PrintTree leaf:" << "   len:" << np->getKey(i)->getLen() << std::endl;
  //   }
  // }
}

void
SITree::Print(string s)
{
#ifdef DEBUG_KVSTORE
  fputs(Util::showtime().c_str(), Util::debug_kvstore);
	fputs("Class SITree\n", Util::debug_kvstore);
	fputs("Message: ", Util::debug_kvstore);
	fputs(s.c_str(), Util::debug_kvstore);
	fputs("\n", Util::debug_kvstore);
	fprintf(Util::debug_kvstore, "Height: %d\n", this->height);
	if (s == "tree" || s == "TREE")
	{
		if (this->root == NULL)
		{
			fputs("Null SITree\n", Util::debug_kvstore);
			return;
		}
		SINode** ns = new SINode*[this->height];
		int* ni = new int[this->height];
		SINode* np;
		int i, pos = 0;
		ns[pos] = this->root;
		ni[pos] = this->root->getNum();
		pos++;
		while (pos > 0)
		{
			np = ns[pos - 1];
			i = ni[pos - 1];
			this->prepare(np);
			if (np->isLeaf() || i < 0)	//LeafSINode or ready IntlNode
			{							//child-num ranges: 0~num
				if (s == "tree")
					np->print("node");
				else
					np->print("NODE");	//print full node-information
				pos--;
				continue;
			}
			else
			{
				ns[pos] = np->getChild(i);
				ni[pos - 1]--;
				ni[pos] = ns[pos]->getNum();
				pos++;
			}
		}
		delete[] ns;
		delete[] ni;
	}
	else if (s == "LEAVES" || s == "leaves")
	{
		SINode* np;
		for (np = this->leaves_head_; np != NULL; np = np->getNext())
		{
			this->prepare(np);
			if (s == "leaves")
				np->print("node");
			else
				np->print("NODE");
		}
	}
	else if (s == "check tree")
	{
		//check the tree, if satisfy B+ definition
		//TODO	
	}
	else;
#endif
}
void
SITree::SetSingleThread(bool _single)
{
  this->if_single_thread = _single;
}

