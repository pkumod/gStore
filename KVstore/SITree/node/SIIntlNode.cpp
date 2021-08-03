/*=============================================================================
# Filename: SIIntlNode.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:40
# Description: achieve functions in SIIntlNode.h
=============================================================================*/

#include "SIIntlNode.h"

using namespace std;


SIIntlNode::SIIntlNode()
{
	memset(childs, 0, sizeof(SINode*) * MAX_CHILD_NUM);
}

SIIntlNode::SIIntlNode(bool isVirtual)	//call father-class's constructor automaticlly
{
	memset(childs, 0, sizeof(SINode*) * MAX_CHILD_NUM);
}


void
SIIntlNode::Virtual()
{
  this->Release();
  this->SetMemFlagFalse();
}

void
SIIntlNode::Normal()
{
  this->AllocKeys();
  this->SetInMem();
}

/**
 *
 * @param _index
 * @return
 */
SINode*
SIIntlNode::GetChild(int _index) const
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index > num)  //num keys, num+1 childs
	{
		return nullptr;
	}
	else
		return childs[_index];
}

bool
SIIntlNode::setChild(SINode* _child, int _index)
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index > num)
	{
		print(string("error in setChild: Invalid index ") + Util::int2string(_index));
		return false;
	}
	this->childs[_index] = _child;

	return true;
}

bool
SIIntlNode::AddChild(SINode* _child, int _index)
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index > num + 1)
	{
		print(string("error in AddChild: Invalid index ") + Util::int2string(_index));
		return false;
	}
	int i;
	for (i = num; i >= _index; --i)	//DEBUG: right bounder!!!
		childs[i + 1] = childs[i];
	childs[_index] = _child;

	return true;
}

bool
SIIntlNode::subChild(int _index)
{
	int num = this->GetKeyNum();
	if (_index < 0 || _index > num)
	{
		print(string("error in subchild: Invalid index ") + Util::int2string(_index));
		return false;
	}
	int i;
	for (i = _index; i < num; ++i) //DEBUG: right bounder!!!
		childs[i] = childs[i + 1];

	return true;
}

unsigned
SIIntlNode::GetSize() const
{
	unsigned sum = INTL_SIZE, num = this->GetKeyNum(), i;
	for (i = 0; i < num; ++i)
		sum += keys[i].getLen();

	return sum;
}

SINode*
SIIntlNode::Split(SINode* _parent, int _index)
{
	int num = this->GetKeyNum();
	SINode* p = new SIIntlNode;		//right child
	p->setHeight(this->getHeight());
	int i, k;
	for (i = MIN_CHILD_NUM, k = 0; i < num; ++i, ++k)
	{
		p->addKey(this->keys + i, k);
      p->AddChild(this->childs[i], k);
      p->AddKeyNum();
	}
  p->AddChild(this->childs[i], k);
	const Bstr* tp = this->keys + MIN_KEY_NUM;
  this->SetKeyNum(MIN_KEY_NUM);
	_parent->addKey(tp, _index);
  _parent->AddChild(p, _index + 1);	//DEBUG(check the index)
  _parent->AddKeyNum();
	_parent->setDirty();
	p->setDirty();
	this->setDirty();

	return p;
}

/**
 *
 * @param _father
 * @param _index
 * @return
 */
SINode*
SIIntlNode::Coalesce(SINode* _father, int _index)
{
	int i, j = _father->GetKeyNum(), k;	//BETTER: unsigned?
	SINode* p;
	int ccase = 0;
	const Bstr* bstr;
	if (_index < j)	//the right neighbor
	{
		p = _father->GetChild(_index + 1);
		k = p->GetKeyNum();
		if ((unsigned)k > MIN_KEY_NUM)
			ccase = 2;
		else				//==MIN_KEY_NUM
			ccase = 1;
	}
	if (_index > 0)	//the left neighbor
	{
		SINode* tp = _father->GetChild(_index - 1);
		unsigned tk = tp->GetKeyNum();
		if (ccase < 2)
		{
			if (ccase == 0)
				ccase = 3;
			if (tk > MIN_KEY_NUM)
				ccase = 4;
		}
		if (ccase > 2)
		{
			p = tp;
			k = tk;
		}
	}
	switch (ccase)
	{
	case 1:					//union right to this
		this->addKey(_father->getKey(_index), this->GetKeyNum());
        this->AddKeyNum();
		for (i = 0; i < k; ++i)
		{
			this->addKey(p->getKey(i), this->GetKeyNum());
          this->AddChild(p->GetChild(i), this->GetKeyNum());
          this->AddKeyNum();
		}
		this->setChild(p->GetChild(i), this->GetKeyNum());
		_father->subKey(_index);
		_father->subChild(_index + 1);
        _father->SubKeyNum();
        p->SetKeyNum(0);
		//delete p;
		break;
	case 2:					//move one form right
		this->addKey(_father->getKey(_index), this->GetKeyNum());
        _father->SetKey(p->getKey(0), _index);
		p->subKey(0);
        this->AddChild(p->GetChild(0), this->GetKeyNum() + 1);
		p->subChild(0);
        this->AddKeyNum();
        p->SubKeyNum();
		break;
	case 3:					//union left to this
		this->addKey(_father->getKey(_index - 1), 0);
        this->AddKeyNum();
		for (i = k; i > 0; --i)
		{
			int t = i - 1;
			this->addKey(p->getKey(t), 0);
          this->AddChild(p->GetChild(i), 0);
          this->AddKeyNum();
		}
        this->AddChild(p->GetChild(0), 0);
		_father->subKey(_index - 1);
		_father->subChild(_index - 1);
        _father->SubKeyNum();
        p->SetKeyNum(0);
		//delete p;
		break;
	case 4:					//move one from left
		bstr = p->getKey(k - 1);
		p->subKey(k - 1);
		this->addKey(_father->getKey(_index - 1), 0);
        _father->SetKey(bstr, _index - 1);
        this->AddChild(p->GetChild(k), 0);
		p->subChild(k);
        this->AddKeyNum();
        p->SubKeyNum();
		break;
	default:
		print("error in Coalesce: Invalid case!");
		//printf("error in Coalesce: Invalid case!");
	}

	_father->setDirty();
	p->setDirty();
	this->setDirty();
	if (ccase == 1 || ccase == 3)
		return p;
	else
		return NULL;
}

void
SIIntlNode::Release()
{
	if (!this->inMem())
		return;
	unsigned num = this->GetKeyNum();
	//delete[] keys;  //this will release all!!!
	for (unsigned i = num; i < MAX_KEY_NUM; ++i)
		keys[i].clear();
	delete[] keys;
}

SIIntlNode::~SIIntlNode()
{
  Release();
	//free(childs);
}

void
SIIntlNode::print(string s)
{
#ifdef DEBUG_KVSTORE
	int num = this->getNum();
	fputs(Util::showtime().c_str(), Util::debug_kvstore);
	fputs("Class SIIntlNode\n", Util::debug_kvstore);
	fputs("Message: ", Util::debug_kvstore);
	fputs(s.c_str(), Util::debug_kvstore);
	fputs("\n", Util::debug_kvstore);
	if (s == "node" || s == "NODE")
	{
		fprintf(Util::debug_kvstore, "store: %u\tnum: %u\tflag: %u\n", this->store, num, this->node_flag_);
		int i;
		for (i = 0; i < num; ++i)
		{
			if (s == "node")
				this->keys[i].print("bstr");
			else
				this->keys[i].print("BSTR");
		}
	}
	else if (s == "check node")
	{
		//TODO(check node, if satisfy B+ definition)
	}
	else;
#endif
}

