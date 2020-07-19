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

//void
//Signature::encodePredicate2Entity(int _pre_id, EntityBitSet& _entity_bs, const char _type)
//{
//if (Signature::PREDICATE_ENCODE_METHOD == 0)
//{
////WARN:change if need to use again, because the encoding method has changed now!
//int pos = ((_pre_id + 10) % Signature::EDGE_SIG_LENGTH) + Signature::STR_SIG_LENGTH;
//_entity_bs.set(pos);
//}
//else
//{
//int seed_num = _pre_id % Signature::EDGE_SIG_INTERVAL_NUM_HALF;

//if (_type == Util::EDGE_OUT)
//{
//seed_num += Signature::EDGE_SIG_INTERVAL_NUM_HALF;
//}

////int primeSize = 5;
////int prime1[]={5003,5009,5011,5021,5023};
////int prime2[]={49943,49957,49991,49993,49999};

////NOTICE: more ones in the bitset(use more primes) means less conflicts, but also weakens the filtration of VSTree.
//// when the data set is big enough, cutting down the size of candidate list should come up to our primary consideration.
//// in this case we should not encode too many ones in entities' signature.
//// also, when the data set is small, hash conflicts can hardly happen.
//// therefore, I think using 2 primes(set up two ones in bitset) is enough.
//// --by hanshuo.
////int primeSize = 2;
////int prime1[] = {5003, 5011};
////int prime2[] = {49957, 49993};

////for(int i = 0; i < primeSize; i++)
////{
////int seed = _pre_id * prime1[i] % prime2[i];
////int pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::STR_SIG_LENGTH + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
////_entity_bs.set(pos);
////}
//int seed = _pre_id * 5003 % 49957;
//int pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::STR_SIG_LENGTH + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
//_entity_bs.set(pos);
//}
//}

//void
//Signature::encodePredicate2Edge(int _pre_id, EdgeBitSet& _edge_bs)
//{
//if (Signature::PREDICATE_ENCODE_METHOD == 0)
//{
//int pos = (_pre_id + 10) % Signature::EDGE_SIG_LENGTH;
//_edge_bs.set(pos);
//}
//else
//{
//int seed_num = _pre_id % Signature::EDGE_SIG_INTERVAL_NUM_HALF;
////int primeSize = 5;
////int prime1[]={5003,5009,5011,5021,5023};
////int prime2[]={49943,49957,49991,49993,49999};

////int primeSize = 2;
////int prime1[] = {5003,5011};
////int prime2[] = {49957,49993};

////for (int i = 0; i < primeSize; i++)
////{
////int seed = _pre_id * prime1[i] % prime2[i];
////int pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
////_edge_bs.set(pos);
////}
//int seed = _pre_id * 5003 % 49957;
//int pos = (seed % Signature::EDGE_SIG_INTERVAL_BASE) + Signature::EDGE_SIG_INTERVAL_BASE * seed_num;
//_edge_bs.set(pos);
//}
//}

////NOTICE: no need to encode itself because only variable in query need to be filtered!
////So only consider all neighbors!
//void
//Signature::encodeStr2Entity(const char* _str, EntityBitSet& _entity_bs)
//{
////_str is subject or object or literal
//if (strlen(_str) >0 && _str[0] == '?')
//return;

//int length = (int)strlen(_str);
//unsigned int hashKey = 0;
//unsigned int pos = 0;
//char *str2 = (char*)calloc(length + 1, sizeof(char));
//strcpy(str2, _str);
//char *str = str2;

//unsigned base = Signature::STR_SIG_BASE * (Signature::HASH_NUM - 1);
//for (int i = Signature::HASH_NUM - 1; i >= 0; --i)
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
////BETTER: use multiple threads for different hash functions

//#ifdef DEBUG_VSTREE
////std::stringstream _ss;
////_ss << "encodeStr2Entity:" << str2 << endl;
////Util::logging(_ss.str());
//#endif
//free(str2);
//}

//void
//Signature::encodeStrID2Entity(int _str_id, EntityBitSet& _entity_bs)
//{
////NO NEED
//}

//TODO: what if pre or neighbor not exist in Query? how to ensure the containment?!
void
Signature::encodeEdge2Entity(EntityBitSet& _entity_bs, int _pre_id, int _neighbor_id, const char _type)
{
  //switch(_type)
  //{
  //case Util::EDGE_IN:
  //break;
  //case Util::EDGE_OUT:
  //break;
  //default:
  //cout<<"error in Signature::encodeEdge2Entity() - non seen type"<<endl;
  //break;
  //}

  int seed_num = _pre_id % Signature::EDGE_SIG_INTERVAL_NUM;
  if (_type == Util::EDGE_OUT) {
    seed_num += Signature::ENTITY_SIG_INTERVAL_HALF;
  }
  if (Util::is_literal_ele(_neighbor_id)) {
    seed_num += Signature::EDGE_SIG_INTERVAL_NUM;
  }

  int base = Signature::ENTITY_SIG_INTERVAL_BASE * seed_num;
  int seed = _pre_id * 5003 % 49957;
  int pos = (seed % Signature::PRE_SIG_BASE) + base;
  _entity_bs.set(pos);
  base += Signature::PRE_SIG_BASE;
  seed = _neighbor_id * 5003 % 49957;
  pos = (seed % Signature::STR_SIG_LENGTH) + base;
  _entity_bs.set(pos);
}

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
