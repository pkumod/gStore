/*=============================================================================
# Filename: SigEntry.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 13:49
# Description: 
=============================================================================*/

#include "SigEntry.h"

using namespace std;

SigEntry::SigEntry()
{
  (this->sig).entityBitSet.reset();
  this->entity_id = INVALID_ENTITY_LITERAL_ID;
  //this->entity_id = -1;
}

SigEntry::SigEntry(TYPE_ENTITY_LITERAL_ID _entity_id, EntityBitSet& _bitset)
{
  this->entity_id = _entity_id;
  (this->sig).entityBitSet |= _bitset;
}

SigEntry::SigEntry(const SigEntry& _sig_entry)
{
  this->sig = _sig_entry.getEntitySig();
  this->entity_id = _sig_entry.entity_id;
}

SigEntry::SigEntry(const EntitySig& _sig, TYPE_ENTITY_LITERAL_ID _entity_id)
{
  this->sig = _sig;
  this->entity_id = _entity_id;
}

const EntitySig&
SigEntry::getEntitySig() const
{
  return this->sig;
}

TYPE_ENTITY_LITERAL_ID
SigEntry::getEntityId() const
{
  return this->entity_id;
}

int
SigEntry::getSigCount() const
{
  return (int)this->sig.entityBitSet.count();
}

SigEntry&
SigEntry::
operator=(const SigEntry _sig_entry)
{
  this->entity_id = _sig_entry.getEntityId();
  this->sig.entityBitSet.reset();
  this->sig.entityBitSet |= _sig_entry.getEntitySig().entityBitSet;
  return *this;
}

SigEntry&
SigEntry::
operator|=(const SigEntry _sig_entry)
{
  const EntitySig& sig = (_sig_entry.getEntitySig());
  (this->sig).entityBitSet |= sig.entityBitSet;
  return *this;
}

bool
SigEntry::cover(const SigEntry& _sig_entry) const
{
  //EQUAL:this & that == that
  return (this->sig.entityBitSet | _sig_entry.getEntitySig().entityBitSet)
         == (this->sig.entityBitSet);
}

bool
SigEntry::cover(const EntitySig& _sig) const
{
  return (this->sig.entityBitSet | _sig.entityBitSet) == (this->sig.entityBitSet);
}

int
SigEntry::xOR(const SigEntry& _sig_entry) const
{
  EntityBitSet entityBitSet;
  entityBitSet.reset();
  entityBitSet |= this->sig.entityBitSet;
  //NOTICE: compute the xor distince now
  //a^b = (a & ~b) | (~a & b)
  EntityBitSet another;
  another.reset();
  another |= _sig_entry.getEntitySig().entityBitSet;
  return ((entityBitSet & another.flip()) | (entityBitSet.flip() & another)).count();
}

//how many 1s in _sig_entry are contained ->flip-> not contained these 1s, as distince
//0s in _sig_entry is nonsense
int
SigEntry::xEpsilen(const SigEntry& _sig_entry) const
{
  EntityBitSet entityBitSet;
  entityBitSet.reset();
  entityBitSet |= this->sig.entityBitSet;
  entityBitSet.flip();
  return (entityBitSet & _sig_entry.getEntitySig().entityBitSet).count();
}

string
SigEntry::to_str() const
{
  std::stringstream _ss;

  _ss << "" << this->entity_id << " : " << Signature::BitSet2str(this->sig.entityBitSet);

  return _ss.str();
}
