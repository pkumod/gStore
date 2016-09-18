/*
 * Signature.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 *  Modified on: 2014-6-29
 *      add some private hash functions,
 *      fix some ill-formed function names.
 *      Author: hanshuo
 */

#ifndef SIGNATURE_H_
#define SIGNATURE_H_

#include<iostream>
#include<string.h>
#include<bitset>
#include<sstream>
using namespace std;

class Signature{
public:
	/* must make sure:
	 *  ENTITY_SIG_LENGTH = EDGE_SIG_LENGTH + STR_SIG_LENGTH */
	const static int ENTITY_SIG_LENGTH = 400;
	const static int EDGE_SIG_LENGTH = 150;
	const static int STR_SIG_LENGTH = 250;

	typedef bitset<Signature::EDGE_SIG_LENGTH> EdgeBitSet;
	typedef bitset<Signature::ENTITY_SIG_LENGTH> EntityBitSet;

	static std::string BitSet2str(const EntityBitSet& _bitset);

	/* there are two predicate encoding method now, see the encoding functions @Signature.cpp for details. */
	const static int PREDICATE_ENCODE_METHOD = 1;
	static void encodePredicate2Entity(int _pre_id, EntityBitSet& _entity_bs, const char _type);
	static void encodePredicate2Edge(int _pre_id, EdgeBitSet& _edge_bs);
	static void encodeStr2Entity(const char* _str, EntityBitSet& _entity_bs); //_str is subject or object(literal)
	static void encodeStrID2Entity(int _str_id, EntityBitSet& _entity_bs);
	unsigned int hash(const char* _str);
private:
	static unsigned int BKDRHash(const char *_str);
	static unsigned int simpleHash(const char *_str);
	static unsigned int RSHash(const char *_str);
	static unsigned int JSHash(const char *_str);
	static unsigned int PJWHash(const char *_str);
	static unsigned int ELFHash(const char *_str);
	static unsigned int SDBMHash(const char *_str);
	static unsigned int DJBHash(const char *_str);
	static unsigned int APHash(const char *_str);
};

typedef bitset<Signature::EDGE_SIG_LENGTH> EdgeBitSet;
typedef bitset<Signature::ENTITY_SIG_LENGTH> EntityBitSet;

class EntitySig : Signature{
public:
    EntityBitSet entityBitSet;
	EntitySig();
	EntitySig(const EntitySig* _p_sig);
	EntitySig(const EntitySig& _sig);
	EntitySig(const EntityBitSet& _bitset);
	EntitySig& operator|=(const EntitySig& _sig);
	bool operator==(const EntitySig& _sig)const;
	bool operator!=(const EntitySig& _sig)const;
	EntitySig& operator=(const EntitySig& _sig);
	const EntityBitSet& getBitset()const;
	void encode(const char * _str, int _pre_id);
};

class EdgeSig : Signature{
public:
    EdgeBitSet edgeBitSet;
	EdgeSig();
	EdgeSig(const EdgeSig* _p_sig);
	EdgeSig(const EdgeSig& _sig);
	EdgeSig(const EdgeBitSet& _bitset);
	EdgeSig& operator|=(const EdgeSig& _sig);
};

#endif /* SIGNATURE_H_ */
