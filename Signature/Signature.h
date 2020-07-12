/*=============================================================================
# Filename: Signature.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-04-11 12:50
# Description: written by liyouhuan and hanshuo
=============================================================================*/

#ifndef _SIGNATURE_SIGNATURE_H
#define _SIGNATURE_SIGNATURE_H

#include "../Util/Util.h"

//NOTICE:
//1. it is hard to set the parameter dynamiclly in Signature
//2. unable to bind an edge's neighbor and pre because either pre or neighbor can be a variable in query
class Signature {
  public:
  //NOTICE: the match can be 160 * 3 + 160 * 2
  //or 200 * 3 + 100 * 2
  //TODO: we should adjust the parameter to harvest the best performance

  //static HashFunction hash[HashNum];
  //must make sure:   ENTITY_SIG_LENGTH = EDGE_SIG_LENGTH + STR_SIG_LENGTH
  //static const unsigned STR_SIG_INTERVAL_NUM = 20;
  //static const unsigned STR_SIG_INTERVAL_NUM = 16;
  //static const unsigned STR_SIG_INTERVAL_BASE = 10;
  //static const unsigned STR_SIG_LITERAL = STR_SIG_INTERVAL_NUM * STR_SIG_INTERVAL_BASE;
  static const int STR_SIG_LITERAL = 200;
  static const unsigned STR_SIG_ENTITY = STR_SIG_LITERAL * 2;
  //here we divide as entity neighbors and literal neighbors: ENTITY(in and out), LITERAL(only for out edges)
  static const unsigned STR_SIG_LENGTH = STR_SIG_ENTITY + STR_SIG_LITERAL; //600

  //NOTICE: after vstree filter, all constant neighbors will be used again to do precise filtering
  //howvere, only few constant pres will be used again for filtering later
  //So we must make most use of the pres here, while keeping the effect of string part
  //(otherwise the result will be too large)

  //str filter is more important in VSTree than predicate, because
  //a predicate may correspond to a lot of entities and predicate num is usually small
  static const unsigned EDGE_SIG_INTERVAL_NUM_HALF = 10; //in edge or out edge
  //static const unsigned EDGE_SIG_INTERVAL_NUM_HALF = 16;   //in edge or out edge
  static const unsigned EDGE_SIG_INTERVAL_NUM = 2 * EDGE_SIG_INTERVAL_NUM_HALF;
  static const unsigned EDGE_SIG_INTERVAL_BASE = 10;
  static const unsigned EDGE_SIG_LENGTH = EDGE_SIG_INTERVAL_NUM * EDGE_SIG_INTERVAL_BASE; //200
  //static const unsigned EDGE_SIG_LENGTH2 = EDGE_SIG_INTERVAL_NUM_HALF * EDGE_SIG_INTERVAL_BASE;

  static const unsigned STR_AND_EDGE_INTERVAL_BASE = 48;
  static const unsigned STR_AND_EDGE_INTERVAL_NUM = 3;
  static const unsigned STR_AND_EDGE_SIG_LENGTH = STR_AND_EDGE_INTERVAL_BASE * STR_AND_EDGE_INTERVAL_NUM; //144
  static const unsigned SEPARATE_SIG_LENGTH = STR_SIG_LENGTH + EDGE_SIG_LENGTH;
  static const unsigned ENTITY_SIG_LENGTH = SEPARATE_SIG_LENGTH + STR_AND_EDGE_SIG_LENGTH; //944

  //static const unsigned ENTITY_SIG_LENGTH = STR_SIG_LENGTH + EDGE_SIG_LENGTH; //1000
  //static const unsigned ENTITY_SIG_LENGTH = STR_SIG_LENGTH + EDGE_SIG_LENGTH + NEIGHBOR_SIG_LENGTH;

  //QUERY: the num of bitset must be based on 16, i.e. unsigned short?  1000 is not allowed
  //but 800, 500 is ok

  //typedef std::bitset<Signature::EDGE_SIG_LENGTH2> EdgeBitSet;
  typedef std::bitset<ENTITY_SIG_LENGTH> EntityBitSet;

  static std::string BitSet2str(const EntityBitSet& _bitset);

  //NOTICE: there are two predicate encoding method now, see the encoding functions @Signature.cpp for details
  const static unsigned PREDICATE_ENCODE_METHOD = 1;
  static void encodePredicate2Entity(EntityBitSet& _entity_bs, TYPE_PREDICATE_ID _pre_id, const char _type);
  static void encodeStr2Entity(EntityBitSet& _entity_bs, TYPE_ENTITY_LITERAL_ID _neighbor_id, const char _type);
  static void encodeEdge2Entity(EntityBitSet& _entity_bs, TYPE_PREDICATE_ID _pre_id, TYPE_ENTITY_LITERAL_ID _neighbor_id, const char _type);
  //static void encodeStrID2Entity(unsigned _str_id, EntityBitSet& _entity_bs);
  //static void encodePredicate2Edge(unsigned _pre_id, EdgeBitSet& _edge_bs);

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
  void encode(const char* _str, TYPE_PREDICATE_ID _pre_id);
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
