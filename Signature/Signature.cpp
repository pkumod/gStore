/*=============================================================================
# Filename: Signature.cpp
# Author: Bookug Lobert
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 13:18
# Description:
=============================================================================*/

#include "Signature.h"

using namespace std;

std::string
Signature::BitSet2str(const EntityBitSet& _bitset)
{
  std::stringstream _ss;
  bool any = false;
  for (unsigned i = 0; i < _bitset.size(); i++) {
    if (_bitset.test(i)) {
      _ss << "[" << i << "] ";
      any = true;
    }
  }
  if (!any) {
    _ss << "empty" << endl;
  }
  _ss << endl;
  return _ss.str();
}

void
Signature::encodeEdge2Entity(EntityBitSet& _entity_bs, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _neighbor_id, const char _type)
{
  Signature::encodePredicate2Entity(_entity_bs, _pre_id, _type);

#ifdef DEBUG
//if(_neighbor_id == 438460)
//{
//cout<<"predicate encoded"<<endl;
//}
#endif

  Signature::encodeStr2Entity(_entity_bs, _neighbor_id, _type);

  //	for(int i = 800; i < _entity_bs.size(); i++){
  //		_entity_bs.set(i);
  //	}
  //encode predicate and entity together
  int x = _pre_id % Signature::STR_AND_EDGE_INTERVAL_BASE;
  int y = _neighbor_id % Signature::STR_AND_EDGE_INTERVAL_BASE;
  int seed = x + (x + y + 1) * (x + y) / 2;
  seed %= Signature::STR_AND_EDGE_INTERVAL_BASE;
  seed = seed + Signature::STR_SIG_LENGTH + Signature::EDGE_SIG_LENGTH;
  if (Util::is_literal_ele(_neighbor_id)) {
    seed += (Signature::STR_AND_EDGE_INTERVAL_BASE * 2);
  } else //entity part
  {
    //entity can be in edge or out edge
    if (_type == Util::EDGE_OUT) {
      seed += Signature::STR_AND_EDGE_INTERVAL_BASE;
    }
  }
  _entity_bs.set(seed);
}

void
Signature::encodePredicate2Entity(EntityBitSet& _entity_bs, TYPE_PREDICATE_ID _pre_id, const char _type)
{
  //NOTICE:this not used now
  if (Signature::PREDICATE_ENCODE_METHOD == 0) {
    //WARN:change if need to use again, because the encoding method has changed now!
    unsigned pos = ((_pre_id + 10) % Signature::EDGE_SIG_LENGTH) + Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
  } else {
    //NOTICE: in * maybe the unsigned will overflow
    long long id = _pre_id;
    unsigned seed_num = id % Signature::EDGE_SIG_INTERVAL_NUM_HALF;

    //int pos = Signature::STR_SIG_LENGTH;
    if (_type == Util::EDGE_OUT) {
      seed_num += Signature::EDGE_SIG_INTERVAL_NUM_HALF;
      //pos += Signature::EDGE_SIG_IN;
    }

    //unsigned primeSize = 5;
    //unsigned prime1[]={5003,5009,5011,5021,5023};
    //unsigned prime2[]={49943,49957,49991,49993,49999};

    //NOTICE: more ones in the bitset(use more primes) means less conflicts, but also weakens the filtration of VSTree.
    // when the data set is big enough, cutting down the size of candidate list should come up to our primary consideration.
    // in this case we should not encode too many ones in entities' signature.
    // also, when the data set is small, hash conflicts can hardly happen.
    // therefore, I think using 2 primes(set up two ones in bitset) is enough.
    // --by hanshuo.
    //unsigned primeSize = 2;
    //unsigned prime1[] = {5003, 5011};
    //unsigned prime2[] = {49957, 49993};

    //for(unsigned i = 0; i < primeSize; i++)
    //{
    //unsigned seed = _pre_id * prime1[i] % prime2[i];
    //unsigned pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::STR_SIG_LENGTH + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
    //_entity_bs.set(pos);
    //}
    //unsigned seed = id * 5003 % 49957;
    //unsigned pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::STR_SIG_LENGTH + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
    //_entity_bs.set(pos);

    long long seed = id * 5003 % 49957;
    seed = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::STR_SIG_LENGTH + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
    _entity_bs.set(seed);
  }
}

//void
//Signature::encodePredicate2Edge(unsigned _pre_id, EdgeBitSet& _edge_bs)
//{
//if (Signature::PREDICATE_ENCODE_METHOD == 0)
//{
//unsigned pos = (_pre_id + 10) % Signature::EDGE_SIG_LENGTH;
//_edge_bs.set(pos);
//}
//else
//{
//unsigned seed_num = _pre_id % Signature::EDGE_SIG_INTERVAL_NUM_HALF;
////unsigned primeSize = 5;
////unsigned prime1[]={5003,5009,5011,5021,5023};
////unsigned prime2[]={49943,49957,49991,49993,49999};

////unsigned primeSize = 2;
////unsigned prime1[] = {5003,5011};
////unsigned prime2[] = {49957,49993};

////for (unsigned i = 0; i < primeSize; i++)
////{
////unsigned seed = _pre_id * prime1[i] % prime2[i];
////unsigned pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
////_edge_bs.set(pos);
////}
//unsigned seed = _pre_id * 5003 % 49957;
//unsigned pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
//_edge_bs.set(pos);
//}
//}

//NOTICE: no need to encode itself because only variable in query need to be filtered!
//So only consider all neighbors!
void
Signature::encodeStr2Entity(EntityBitSet& _entity_bs, TYPE_ENTITY_LITERAL_ID _neighbor_id, const char _type)
{
  //NOTICE: we assume the parameter is always valid(invalid args should not be passed here)
  long long id = _neighbor_id;
  //NOTICE: in * maybe the unsigned will overflow
  //long long seed = id * 5003 % 49957;
  //seed = seed % Signature::STR_SIG_INTERVAL_BASE;
  //seed = seed + (id % Signature::STR_SIG_INTERVAL_NUM) * Signature::STR_SIG_INTERVAL_BASE;

  int seed = _neighbor_id % Signature::STR_SIG_LITERAL;

  if (Util::is_literal_ele(_neighbor_id)) {
    seed += Signature::STR_SIG_ENTITY;
  } else //entity part
  {
    //entity can be in edge or out edge
    if (_type == Util::EDGE_OUT) {
      seed += Signature::STR_SIG_LITERAL;
    }
  }

  //if(_neighbor_id == 438460)
  //{
  //cout<<_neighbor_id<<" "<<seed<<endl;
  //}

  _entity_bs.set(seed);

//_str is subject or object or literal
//if (strlen(_str) >0 && _str[0] == '?')
//return;
//unsigned length = (unsigned)strlen(_str);
//unsigned unsigned hashKey = 0;
//unsigned unsigned pos = 0;
//char *str2 = (char*)calloc(length + 1, sizeof(char));
//strcpy(str2, _str);
//char *str = str2;
//unsigned base = Signature::STR_SIG_BASE * (Signature::HASH_NUM - 1);
//for (unsigned i = Signature::HASH_NUM - 1; i >= 0; --i)
//{
//HashFunction hf = Util::hash[i];
//if (hf == NULL)
//break;
//hashKey = hf(str);
//str = str2;
//pos = base + hashKey % Signature::STR_SIG_BASE;
//base -= Signature::STR_SIG_BASE;
//if (_str[0] == '"')
//{
//pos += Signature::STR_SIG_LENGTH2;
//}
//else if (_str[0] != '<')
//{
//#ifdef DEBUG_VSTREE
//cerr << "error in encodeStr2Entity(): neighbor is neither a literal or entity!" << endl;
//#endif
//}
//_entity_bs.set(pos);
//}
//BETTER: use multiple threads for different hash functions

#ifdef DEBUG_VSTREE
//std::stringstream _ss;
//_ss << "encodeStr2Entity:" << str2 << endl;
//Util::logging(_ss.str());
#endif
  //free(str2);
}

//void
//Signature::encodeStrID2Entity(unsigned _str_id, EntityBitSet& _entity_bs)
//{
////NOT USED NOW
//}

EntitySig::EntitySig()
{
  this->entityBitSet.reset();
}

EntitySig::EntitySig(const EntitySig* _p_sig)
{
  this->entityBitSet.reset();
  this->entityBitSet |= _p_sig->entityBitSet;
}

EntitySig::EntitySig(const EntitySig& _sig)
{
  this->entityBitSet.reset();
  this->entityBitSet |= _sig.entityBitSet;
}

EntitySig::EntitySig(const EntityBitSet& _bitset)
{
  this->entityBitSet.reset();
  this->entityBitSet |= _bitset;
}

EntitySig&
EntitySig::
operator|=(const EntitySig& _sig)
{
  this->entityBitSet |= _sig.entityBitSet;
  return *this;
}

bool
EntitySig::
operator==(const EntitySig& _sig) const
{
  return (this->entityBitSet == _sig.entityBitSet);
}

bool
EntitySig::
operator!=(const EntitySig& _sig) const
{
  return (this->entityBitSet != _sig.entityBitSet);
}

EntitySig&
EntitySig::
operator=(const EntitySig& _sig)
{
  this->entityBitSet.reset();
  this->entityBitSet |= _sig.getBitset();
  return *this;
}

const EntityBitSet&
EntitySig::getBitset() const
{
  return this->entityBitSet;
}

//EdgeSig::EdgeSig()
//{
//this->edgeBitSet.reset();
//}

//EdgeSig::EdgeSig(const EdgeSig* _p_sig)
//{
//this->edgeBitSet.reset();
//this->edgeBitSet |= _p_sig->edgeBitSet;
//}

//EdgeSig::EdgeSig(const EdgeSig& _sig)
//{
//this->edgeBitSet.reset();
//this->edgeBitSet |= _sig.edgeBitSet;
//}

//EdgeSig::EdgeSig(const EdgeBitSet& _bitset)
//{
//this->edgeBitSet.reset();
//this->edgeBitSet |= _bitset;
//}

//EdgeSig&
//EdgeSig::operator|=(const EdgeSig& _sig)
//{
//this->edgeBitSet |= _sig.edgeBitSet;
//return *this;
//}

string
EntitySig::to_str() const
{
  std::stringstream _ss;

  _ss << Signature::BitSet2str(this->entityBitSet);

  return _ss.str();
}
