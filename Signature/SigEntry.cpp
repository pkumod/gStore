/*
 * SIGEntry.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 *  Modified on: 2014-6-29
 *      Author: hanshuo
 */

#include "SigEntry.h"

SigEntry::SigEntry()
{
	(this->sig).entityBitSet.reset();
    this->entity_id = -1;
}

SigEntry::SigEntry(int _entity_id, EntityBitSet& _bitset)
{
	this->entity_id = _entity_id;
	(this->sig).entityBitSet |= _bitset;
}

SigEntry::SigEntry(const SigEntry& _sig_entry)
{
    this->sig = _sig_entry.getEntitySig();
    this->entity_id = _sig_entry.entity_id;
}

SigEntry::SigEntry(const EntitySig& _sig, int _entity_id)
{
	this->sig = _sig;
	this->entity_id = _entity_id;
}

const EntitySig& SigEntry::getEntitySig()const
{
	return this->sig;
}

int SigEntry::getEntityId()const
{
    return this->entity_id;
}

int SigEntry::getSigCount()const
{
    return (int)this->sig.entityBitSet.count();
}

SigEntry& SigEntry::operator=(const SigEntry _sig_entry)
{
	this->entity_id = _sig_entry.getEntityId();
	this->sig.entityBitSet.reset();
	this->sig.entityBitSet |= _sig_entry.getEntitySig().entityBitSet;
	return *this;
}

SigEntry& SigEntry::operator|=(const SigEntry _sig_entry)
{
	const EntitySig& sig = (_sig_entry.getEntitySig());
	(this->sig).entityBitSet |= sig.entityBitSet;
	return *this;
}

bool SigEntry::cover(const SigEntry& _sig_entry)const
{
    return (this->sig.entityBitSet | _sig_entry.getEntitySig().entityBitSet)
    		== (this->sig.entityBitSet);
}

bool SigEntry::cover(const EntitySig& _sig)const
{
    return (this->sig.entityBitSet | _sig.entityBitSet) == (this->sig.entityBitSet);
}

int SigEntry::xEpsilen(const SigEntry& _sig_entry)const
{
    EntityBitSet entityBitSet;
    entityBitSet.reset();
    entityBitSet |= this->sig.entityBitSet;
    entityBitSet.flip();
    return (entityBitSet & _sig_entry.getEntitySig().entityBitSet).count();
}

std::string SigEntry::to_str()const
{
	std::stringstream _ss;

	_ss << "" << this->entity_id << " : " <<
		Signature::BitSet2str(this->sig.entityBitSet);

	return _ss.str();
}


