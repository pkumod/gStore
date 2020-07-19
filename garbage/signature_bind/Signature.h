/*=============================================================================
# Filename: Signature.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 12:50
# Description: written by liyouhuan and hanshuo, modified by zengli
=============================================================================*/

#ifndef _SIGNATURE_SIGNATURE_H
#define _SIGNATURE_SIGNATURE_H

#include "../Util/Util.h"

class Signature {
  public:
  //TODO:how to set the length as a dynamic parameter?    below use typedef bitset<num>
  //keep the parameter in Database and passed to here and Query??
  //QUERY: is this really needed?!

  //static HashFunction hash[HashNum];
  static const int PRE_SIG_BASE = 10;
  static const int STR_SIG_BASE = PRE_SIG_BASE;
  static const int STR_SIG_LENGTH = 3 * STR_SIG_BASE;
  static const int ENTITY_SIG_INTERVAL_BASE = PRE_SIG_BASE + STR_SIG_LENGTH;
  static const int EDGE_SIG_INTERVAL_NUM = 5; //in edge or out edge, entity or literal
  static const int ENTITY_SIG_INTERVAL_HALF = 2 * EDGE_SIG_INTERVAL_NUM;
  //STRUCT: in egde | out egde(neighbor is entity | literal(pre1 pre2 pre3 pre4 pre5(str pre * 3) ) )
  //NOTICE: the length should below 1000
  static const int ENTITY_SIG_LENGTH = 2 * 2 * EDGE_SIG_INTERVAL_NUM * ENTITY_SIG_INTERVAL_BASE;

  //NOTICE: we can also use id here, but string is recommended due to special structure
  //(maybe needed later, for example, wildcards)
  //Th ehash function is costly, so just use two
  //static const int HASH_NUM = 3; //no more than Util::HashNum
  //NOTICE:if using str id, we can also divide like EDGE_SIG
  //here we divide as entity neighbors and literal neighbors: ENTITY, LITERAL
  //static const int STR_SIG_LENGTH = 2 * STR_SIG_BASE * HASH_NUM;    //250
  //static const int STR_SIG_LENGTH2 = STR_SIG_BASE * HASH_NUM;

  //QUERY:I think that str filter is more important in VSTree than predicate, because
  //a predicate may correspond to a lot of entities and predicate num is usually small
  //static const int EDGE_SIG_INTERVAL_NUM_HALF = 5;   //in edge or out edge
  //static const int EDGE_SIG_INTERVAL_NUM = 2 * EDGE_SIG_INTERVAL_NUM_HALF;
  //static const int EDGE_SIG_INTERVAL_BASE = 20;
  //static const int EDGE_SIG_LENGTH = EDGE_SIG_INTERVAL_NUM * EDGE_SIG_INTERVAL_BASE; //150
  //static const int EDGE_SIG_LENGTH2 = EDGE_SIG_INTERVAL_NUM_HALF * EDGE_SIG_INTERVAL_BASE; //150

  //static const int ENTITY_SIG_LENGTH = STR_SIG_LENGTH + EDGE_SIG_LENGTH;
  //static const int ENTITY_SIG_LENGTH = STR_SIG_LENGTH + EDGE_SIG_LENGTH + NEIGHBOR_SIG_LENGTH;

  //typedef std::bitset<Signature::EDGE_SIG_LENGTH2> EdgeBitSet;
  typedef std::bitset<Signature::ENTITY_SIG_LENGTH> EntityBitSet;

  static std::string BitSet2str(const EntityBitSet& _bitset);

  //NOTICE: there are two predicate encoding method now, see the encoding functions @Signature.cpp for details
  //const static int PREDICATE_ENCODE_METHOD = 1;
  //static void encodePredicate2Entity(int _pre_id, EntityBitSet& _entity_bs, const char _type);
  //static void encodePredicate2Edge(int _pre_id, EdgeBitSet& _edge_bs);
  //static void encodeStr2Entity(const char* _str, EntityBitSet& _entity_bs); //_str is subject or object(literal)
  //static void encodeStrID2Entity(int _str_id, EntityBitSet& _entity_bs);
  static void encodeEdge2Entity(EntityBitSet& _entity_bs, int _pre_id, int _neighbor_id, const char _type);
  //Signature()
  //{
  //NOTICE:not exceed the HashNum
  //this->hash = new HashFunction[HashNum];
  //this->hash[0] = Util::simpleHash;
  //this->hash[1] = Util::APHash;
  //this->hash[2] = Util::BKDRHash;
  //this->hash[3] = Util::DJBHash;
  //this->hash[4] = Util::ELFHash;
  //this->hash[5] = Util::DEKHash;
  //this->hash[6] = Util::BPHash;
  //this->hash[7] = Util::FNVHash;
  //this->hash[8] = Util::HFLPHash;
  //this->hash[9] = Util::HFHash;
  //this->hash[10] = Util::JSHash;
  //this->hash[11] = Util::PJWHash;
  //this->hash[12] = Util::RSHash;
  //this->hash[13] = Util::SDBMHash;
  //this->hash[14] = Util::StrHash;
  //this->hash[15] = Util::TianlHash;
  //}
  //~Signature()
  //{
  //delete[] this->hash;
  //}
};

//WARN:also defined in Signature, must be same!!!
//NOTICE:EdgeBitSet is only used in Query, not for VSTree
//typedef std::bitset<Signature::EDGE_SIG_LENGTH2> EdgeBitSet;
typedef std::bitset<Signature::ENTITY_SIG_LENGTH> EntityBitSet;

class EntitySig : Signature {
  public:
  EntityBitSet entityBitSet;
  EntitySig();
  EntitySig(const EntitySig* _p_sig);
  EntitySig(const EntitySig& _sig);
  EntitySig(const EntityBitSet& _bitset);
  EntitySig& operator|=(const EntitySig& _sig);
  bool operator==(const EntitySig& _sig) const;
  bool operator!=(const EntitySig& _sig) const;
  EntitySig& operator=(const EntitySig& _sig);
  const EntityBitSet& getBitset() const;
  void encode(const char* _str, int _pre_id);
  std::string to_str() const;
};

//class EdgeSig : Signature{
//public:
//EdgeBitSet edgeBitSet;
//EdgeSig();
//EdgeSig(const EdgeSig* _p_sig);
//EdgeSig(const EdgeSig& _sig);
//EdgeSig(const EdgeBitSet& _bitset);
//EdgeSig& operator|=(const EdgeSig& _sig);
//};

#endif // _SIGNATURE_SIGNATURE_H
