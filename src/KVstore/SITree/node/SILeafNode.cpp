/*=============================================================================
# Filename: SILeafNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: ahieve functions in SILeafNode.h
=============================================================================*/

#include "SILeafNode.h"

using namespace std;

void
SILeafNode::AllocValues()
{
  values = new unsigned[MAX_KEY_NUM];
}

SILeafNode::SILeafNode():SINode()
{
  node_flag_ |= NF_IL;		//leaf node_flag_
  prev = next = nullptr;
  AllocValues();
}

SILeafNode::SILeafNode(bool isVirtual):SINode(isVirtual) //The constructor of father's class must be called explicitly
{
  node_flag_ |= NF_IL;
  prev = next = nullptr;
  values = nullptr;
  if (!isVirtual)
    AllocValues();
}

void
SILeafNode::Virtual()
{
  this->Release();
  this->SetMemFlagFalse();
}

void
SILeafNode::Normal()
{
  if (!keys) this->AllocKeys();
  if (!values) this->AllocValues();
  this->SetInMem();
}

SINode*
SILeafNode::GetPrev() const
{
  return prev;
}

SINode*
SILeafNode::GetNext() const
{
  return next;
}

/**
 * get key[_index] corresponding ID
 * @param _index
 * @return ID if _index is legal, -1 if illegal
 */
unsigned
SILeafNode::GetValue(int _index) const
{
  int num = this->GetKeyNum();
  if (_index < 0 || _index >= num)
  {
    //print(string("error in GetValue: Invalid index ") + Util::int2string(_index));
    SLOG_ERROR("error GetValue:" << _index);
    return 0;
  }
  else
    return this->values[_index];
}

/**
 * set key[_index] corresponding ID
 * @param _val corresponding ID
 * @param _index key's position
 * @return bool
 */
bool
SILeafNode::SetValue(unsigned _val, int _index)
{
  int num = this->GetKeyNum();
  if (_index < 0 || _index >= num)
  {
    print(string("error in SetValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  this->values[_index] = _val;

  return true;
}

/**
 * move values[>= _index] one position rightward
 * and set key[_index] as _val
 * @param _val the corresponding ID
 * @param _index the position
 */
bool
SILeafNode::AddValue(unsigned _val, int _index)
{
  int num = this->GetKeyNum();
  if (_index < 0 || _index > num)
  {
    print(string("error in AddValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  for (i = num - 1; i >= _index; --i)
    this->values[i + 1] = this->values[i];
  this->values[_index] = _val;

  return true;
}

/**
 * key[>index] one step leftward
 * @param _index the deleted position
 */
bool
SILeafNode::SubValue(int _index)
{
  int num = this->GetKeyNum();
  if (_index < 0 || _index >= num)
  {
    print(string("error in SubValue: Invalid index ") + Util::int2string(_index));
    return false;
  }
  int i;
  for (i = _index; i < num - 1; ++i)
    this->values[i] = this->values[i + 1];

  return true;
}

void
SILeafNode::setPrev(SINode* _prev)
{
  this->prev = _prev;
}

void
SILeafNode::SetNext(SINode* _next)
{
  this->next = _next;
}

unsigned
SILeafNode::GetSize() const
{
  unsigned sum = LEAF_SIZE, num = this->GetKeyNum(), i;
  for (i = 0; i < num; ++i)
  {
    sum += keys[i].getLen();
  }
  return sum;
}

/**
 * Split the leaf node, the new node will have
 * (CurrentKeyNum - MIN_CHILD_NUM) keys and placed
 * at the right of this node
 * @param _parent pointer to father node
 * @param _index this node's position in parent node
 * @return the new created node
 */
SINode*
SILeafNode::Split(SINode* _parent, int _index)
{
  int num = this->GetKeyNum();
  SINode* p = new SILeafNode;		//right child

  // NOTICE: assign height for new node
  p->setHeight(this->getHeight());

  if (this->next != nullptr)
  {
    this->next->setPrev(p);
    p->SetNext(this->next);
  }
  this->SetNext(p);
  p->setPrev(this);

  int i, k;
  for (i = MIN_KEY_NUM, k = 0; i < num; ++i, ++k)
  {
    p->addKey(this->keys + i, k);
    p->AddValue(this->values[i], k);
    p->AddKeyNum();
  }
  const Bstr* tp = this->keys + MIN_KEY_NUM;
  this->SetKeyNum(MIN_KEY_NUM);
  _parent->addKey(tp, _index, true);
  // from these code , we can assure k[i] is value[i+1]'s min-value
  _parent->AddChild(p, _index + 1);

  _parent->AddKeyNum();
  _parent->setDirty();
  p->setDirty();
  this->setDirty();

  return p;
}

/**
 * add a key or Coalesce a neighbor to this.
 * Four case
 * 1:union right to this.
 * 2:move one from right
 * 3:union left to this
 * 4:move one from left
 * @param _parent parent node
 * @param _index which position this node is in parent's child
 * @return  neighbour SINode in case 1/3, NULL case 2/4.
 */
SINode*
SILeafNode::Coalesce(SINode* _parent, int _index)
{
  int i, parent_key_num = _parent->GetKeyNum();
  unsigned int neighbour_key_num;
  SINode* neighbour = nullptr;

  // 1:union right to this
  // 2:move one from right
  // 3:union left to this
  // 4:move one from left
  int coalesce_method = 0;
  const Bstr* bstr;

  //it has a right neighbor
  if (_index < parent_key_num)
  {
    neighbour = _parent->GetChild(_index + 1);
    neighbour_key_num = neighbour->GetKeyNum();
    if ((unsigned)neighbour_key_num > MIN_KEY_NUM)
      coalesce_method = 2;
    else // if the right child has <= MIN_KEY_NUM values
      coalesce_method = 1;
  }

  //it has a left neighbor
  if (_index > 0)
  {
    SINode* left_neighbour = _parent->GetChild(_index - 1);
    unsigned tk = left_neighbour->GetKeyNum();
    if (coalesce_method < 2)
    {
      // if not have right neighbour
      if (coalesce_method == 0)
        coalesce_method = 3;
      if (tk > MIN_KEY_NUM)
        coalesce_method = 4;
    }
    if (coalesce_method > 2)
    {
      neighbour = left_neighbour;
      neighbour_key_num = tk;
    }
  }

  if (neighbour == nullptr)
  {
    SLOG_ERROR("error Coalesce neighbour is null:");
    return NULL;
  }
  switch (coalesce_method)
  {
    //union right to this
    case 1:
      for (i = 0; i < static_cast<int>(neighbour_key_num); ++i)
      {
        this->addKey(neighbour->getKey(i), this->GetKeyNum());
        this->AddValue(neighbour->GetValue(i), this->GetKeyNum());
        this->AddKeyNum();
      }
      _parent->subKey(_index, true);
      _parent->subChild(_index + 1);
      _parent->SubKeyNum();
      this->next = neighbour->GetNext();
      if (this->next != nullptr)
        this->next->setPrev(this);
      neighbour->SetKeyNum(0);
      break;

    // move one from right and add it to this node's last position
    // need to adjust keys of parent
    case 2:
      this->addKey(neighbour->getKey(0), this->GetKeyNum());
      _parent->SetKey(neighbour->getKey(1), _index, true);
      neighbour->subKey(0);
      this->AddValue(neighbour->GetValue(0), this->GetKeyNum());
      neighbour->SubValue(0);
      this->AddKeyNum();
      neighbour->SubKeyNum();
      break;

    //union left to this
    case 3:
      // may have efficiency problem
      for (i = neighbour_key_num; i > 0; --i)
      {
        int t = i - 1;
        this->addKey(neighbour->getKey(t), 0);
        this->AddValue(neighbour->GetValue(t), 0);
        this->AddKeyNum();
      }
      _parent->subKey(_index - 1, true);
      _parent->subChild(_index - 1);
      _parent->SubKeyNum();
      this->prev = neighbour->GetPrev();
      if (this->prev != nullptr)
        this->prev->SetNext(this);
      neighbour->SetKeyNum(0);
      break;

    //move one from left
    case 4:
      bstr = neighbour->getKey(neighbour_key_num - 1);
      neighbour->subKey(neighbour_key_num - 1);
      this->addKey(bstr, 0);
      _parent->SetKey(bstr, _index - 1, true);
      this->AddValue(neighbour->GetValue(neighbour_key_num - 1), 0);
      neighbour->SubValue(neighbour_key_num - 1);
      this->AddKeyNum();
      neighbour->SubKeyNum();
      break;
    default:
      print("error in Coalesce: Invalid case!");
  }

  _parent->setDirty();
  neighbour->setDirty();
  this->setDirty();
  if (coalesce_method == 1 || coalesce_method == 3)
    return neighbour;
  else
    return NULL;
}

void
SILeafNode::Release()
{
  if (!this->inMem())
    return;
  unsigned num = this->GetKeyNum();
  for (unsigned i = num; i < MAX_KEY_NUM; ++i)
  {
    keys[i].clear();
  }
  delete[] keys;
  delete[] values;
  keys = nullptr;
  values = nullptr;
}

SILeafNode::~SILeafNode()
{
  Release();
}

void
SILeafNode::print(string s)
{
#ifdef DEBUG_KVSTORE
  unsigned num = this->getNum();
	fputs(Util::showtime().c_str(), Util::debug_kvstore);
	fputs("Class SILeafNode\n", Util::debug_kvstore);
	fputs("Message: ", Util::debug_kvstore);
	fputs(s.c_str(), Util::debug_kvstore);
	fputs("\n", Util::debug_kvstore);
	unsigned i;
	if (s == "NODE")
	{
		fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tnode_flag_: %u\n", this->store, num, this->node_flag_);
		fprintf(Util::debug_kvstore, "prev: %p\tnext: %p\n", this->prev, this->next);
		for (i = 0; i < num; ++i)
		{
			this->keys[i].print("BSTR");
			//this->values[i].print("BSTR");
		}
	}
	else if (s == "node")
	{
		fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tnode_flag_: %u\n", this->store, num, this->node_flag_);
		fprintf(Util::debug_kvstore, "prev: %p\tnext: %p\n", this->prev, this->next);
	}
	else if (s == "check node")
	{
		//check the node, if satisfy B+ definition
		bool node_flag_ = true;
		if (num < MIN_KEY_NUM || num > MAX_KEY_NUM)
			node_flag_ = false;
		if (node_flag_)
		{
			for (i = 1; i < num; ++i)
			{
				if (keys[i] > keys[i - 1])
					continue;
				else
					break;
			}
			if (i < num)
				node_flag_ = false;
		}
		this->print("node");
		if (node_flag_)
			fprintf(Util::debug_kvstore, "This node is good\n");
		else
			fprintf(Util::debug_kvstore, "This node is bad\n");
	}
	else;
#endif
}

