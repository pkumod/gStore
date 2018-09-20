/*=============================================================================
# Filename: TPSelectivity.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Description: implement functions in TPSelectivity.h
=============================================================================*/

#include "TPSelectivity.h"

using namespace std;

inline unsigned TPSelectivity::getSP2O(unsigned sid, unsigned pid)
{
	pair<unsigned, unsigned> key(sid, pid);

	if (this->sp2o_.count(key) == 0)
		this->sp2o_[key] = this->kvstore_->getSubjectPredicateDegree(sid, pid);

	return this->sp2o_[key];
}

inline unsigned TPSelectivity::getOP2S(unsigned oid, unsigned pid)
{
	pair<unsigned, unsigned> key(oid, pid);

	if (this->op2s_.count(key) == 0)
		this->op2s_[key] = this->kvstore_->getObjectPredicateDegree(oid, pid);

	return this->op2s_[key];
}

inline unsigned TPSelectivity::getP2SO(unsigned pid)
{
	if (this->p2so_.count(pid) == 0)
		this->p2so_[pid] = this->kvstore_->getPredicateDegree(pid);

	return this->p2so_[pid];
}

unsigned TPSelectivity::getTPSelectivity(const QueryTree::GroupPattern::Pattern& pattern)
{
	if (pattern.predicate.isVariable())
		return UINT_MAX;

	unsigned pid = this->kvstore_->getIDByPredicate(pattern.predicate.getValue());

	if (pattern.subject.isVariable() && !pattern.object.isVariable())
	{
		unsigned oid = this->kvstore_->getIDByString(pattern.object.getValue());
		return this->getOP2S(oid, pid);
	}
	else if (!pattern.subject.isVariable() && pattern.object.isVariable())
	{
		unsigned sid = this->kvstore_->getIDByEntity(pattern.subject.getValue());
		return this->getSP2O(sid, pid);
	}
	else if (pattern.subject.isVariable() && pattern.object.isVariable())
	{
		return this->getP2SO(pid);
	}
	else
		return 0;
}
