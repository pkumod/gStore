/*
 * Signature.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 *  Implemented on: 2014-6-29
 *      Author: hanshuo
 */

#include "Signature.h"
#include "../Query/BasicQuery.h"
#include "../Database/Database.h"

std::string Signature::BitSet2str(const EntityBitSet& _bitset)
{
	std::stringstream _ss;
	bool any = false;
	for(int i = 0; i < _bitset.size(); i ++)
	{
		if(_bitset.test(i))
		{
			_ss << "[" << i << "] ";
			any = true;
		}
	}
	if(!any)
	{
		_ss << "empty" << endl;
	}
	_ss << endl;
	return _ss.str();
}

/* for Signature */
void Signature::encodePredicate2Entity(int _pre_id, EntityBitSet& _entity_bs, const char _type)
{
    if (Signature::PREDICATE_ENCODE_METHOD == 0)
    {
        int pos = ( (_pre_id+10) % Signature::EDGE_SIG_LENGTH ) + Signature::STR_SIG_LENGTH;
        _entity_bs.set(pos);
    }
    else
    {
        int seed_preid = _pre_id;

        if(_type == BasicQuery::EDGE_OUT)
        {
        	seed_preid += 101;
        }

        /*
        int primeSize = 5;
        int prime1[]={5003,5009,5011,5021,5023};
        int prime2[]={49943,49957,49991,49993,49999};
        */

        // how to hash the predicate id to signature(bitset) better?
        // more ones in the bitset(use more primes) means less conflicts, but also weakens the filtration of VSTree.
        // when the data set is big enough, cutting down the size of candidate list should come up to our primary consideration.
        // in this case we should not encode too many ones in entities' signature.
        // also, when the data set is small, hash conflicts can hardly happen.
        // therefore, I think using 2 primes(set up two ones in bitset) is enough.
        // --by hanshuo.
        int primeSize = 2;
        int prime1[]={5003,5011};
        int prime2[]={49957,49993};

        for (int i=0;i<primeSize;i++)
        {
            int seed = seed_preid * prime1[i] % prime2[i];
            int pos = (seed % Signature::EDGE_SIG_LENGTH ) + Signature::STR_SIG_LENGTH;
            _entity_bs.set(pos);
        }
    }
}

void Signature::encodePredicate2Edge(int _pre_id, EdgeBitSet& _edge_bs)
{
    if (Signature::PREDICATE_ENCODE_METHOD == 0)
    {
        int pos = (_pre_id + 10) % Signature::EDGE_SIG_LENGTH;
        _edge_bs.set(pos);
    }
    else
    {
        /*
        int primeSize = 5;
        int prime1[]={5003,5009,5011,5021,5023};
        int prime2[]={49943,49957,49991,49993,49999};
        */

        int primeSize = 2;
        int prime1[]={5003,5011};
        int prime2[]={49957,49993};

        for (int i=0;i<primeSize;i++)
        {
            int seed = _pre_id * prime1[i] % prime2[i];
            int pos = seed % Signature::EDGE_SIG_LENGTH;
            _edge_bs.set(pos);
        }
    }
}

void Signature::encodeStr2Entity(const char* _str, EntityBitSet& _entity_bs) //_str is subject or object or literal
{
    if(strlen(_str) >0 && _str[0] == '?')
        return;

    int length = (int)strlen(_str);
    unsigned int hashKey = 0;
    unsigned int pos = 0;
    char *str2 = new char[length+1];
    strcpy(str2, _str);
    char *str = str2;

    // the same consideration as encodePredicate2Entity.
    // I think we should not set too many ones in entities' signature.
    hashKey = Signature::simpleHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    str=str2;
    hashKey = Signature::RSHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    str=str2;
    hashKey = Signature::JSHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    str=str2;
    hashKey = Signature::PJWHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    /*
    str=str2;
    hashKey = Signature::ELFHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    str=str2;
    hashKey = Signature::SDBMHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    hashKey = Signature::DJBHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    str=str2;
    hashKey = Signature::APHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    str=str2;
    hashKey = Signature::BKDRHash(str);
    pos = hashKey % Signature::STR_SIG_LENGTH;
    _entity_bs.set(pos);
    */

    /*
    //debug
    {
        std::stringstream _ss;
        _ss << "encodeStr2Entity:" << str2 << endl;
        Database::log(_ss.str());
    }
    */

    delete []str2;
}

void Signature::encodeStrID2Entity(int _str_id, EntityBitSet& _entity_bs)
{
    //to be implement
}

unsigned int Signature::hash(const char* _str)
{
    //to be implement
    return 0;
}

/* some string hash functions */
unsigned int Signature::BKDRHash(const char *_str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int key = 0;

    while (*_str)
    {
        key = key * seed + (*_str++);
    }

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::simpleHash(const char *_str)
{
    unsigned int key;
    unsigned char *p;

    for(key = 0, p = (unsigned char *)_str; *p ; p++)
        key = 31 * key + *p;

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::RSHash(const char *_str)
{
    unsigned int b = 378551;
    unsigned int a = 63689;
    unsigned int key = 0;

    while (*_str)
    {
        key = key * a + (*_str++);
        a *= b;
    }

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::JSHash(const char *_str)
{
    unsigned int key = 1315423911;

    while (*_str)
    {
        key ^= ((key << 5) + (*_str++) + (key >> 2));
    }

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::PJWHash(const char *_str)
{
    unsigned int bits_in_unsigned_int = (unsigned int)(sizeof(unsigned int) * 8);
    unsigned int three_quarters = (unsigned int)((bits_in_unsigned_int * 3) / 4);
    unsigned int one_eighth = (unsigned int)(bits_in_unsigned_int / 8);

    unsigned int high_bits = (unsigned int)(0xFFFFFFFF) << (bits_in_unsigned_int - one_eighth);
    unsigned int key = 0;
    unsigned int test = 0;

    while (*_str)
    {
        key = (key << one_eighth) + (*_str++);
        if ((test = key & high_bits) != 0)
        {
            key = ((key ^ (test >> three_quarters)) & (~high_bits));
        }
    }

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::ELFHash(const char *_str)
{
    unsigned int key = 0;
    unsigned int x  = 0;

    while (*_str)
    {
        key = (key << 4) + (*_str++);
        if ((x = key & 0xF0000000L) != 0)
        {
            key ^= (x >> 24);
            key &= ~x;
        }
    }

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::SDBMHash(const char *_str)
{
    unsigned int key = 0;

    while (*_str)
    {
        key = (*_str++) + (key << 6) + (key << 16) - key;
    }

    return (key & 0x7FFFFFFF);
}

unsigned int Signature::DJBHash(const char *_str)
{
    unsigned int key = 5381;
    while (*_str) {
        key += (key << 5) + (*_str++);
    }
    return (key & 0x7FFFFFFF);
}

unsigned int Signature::APHash(const char *_str)
{
    unsigned int key = 0;
    int i;

    for (i=0; *_str; i++)
    {
        if ((i & 1) == 0)
        {
            key ^= ((key << 7) ^ (*_str++) ^ (key >> 3));
        }
        else
        {
            key ^= (~((key << 11) ^ (*_str++) ^ (key >> 5)));
        }
    }

    return (key & 0x7FFFFFFF);
}

/* for ENTITYsig */
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

EntitySig& EntitySig::operator|=(const EntitySig& _sig)
{
	this->entityBitSet |= _sig.entityBitSet;
	return *this;
}

bool EntitySig::operator==(const EntitySig& _sig)const
{
    return (this->entityBitSet == _sig.entityBitSet);
}

bool EntitySig::operator!=(const EntitySig& _sig)const
{
    return (this->entityBitSet != _sig.entityBitSet);
}

EntitySig& EntitySig::operator=(const EntitySig& _sig)
{
	this->entityBitSet.reset();
	this->entityBitSet |= _sig.getBitset();
	return *this;
}

const EntityBitSet & EntitySig::getBitset()const
{
	return this->entityBitSet;
}

/* for EDGEsig */
EdgeSig::EdgeSig()
{
	this->edgeBitSet.reset();
}
EdgeSig::EdgeSig(const EdgeSig* _p_sig)
{
	this->edgeBitSet.reset();
	this->edgeBitSet |= _p_sig->edgeBitSet;
}
EdgeSig::EdgeSig(const EdgeSig& _sig)
{
	this->edgeBitSet.reset();
	this->edgeBitSet |= _sig.edgeBitSet;
}

EdgeSig::EdgeSig(const EdgeBitSet& _bitset)
{
	this->edgeBitSet.reset();
	this->edgeBitSet |= _bitset;
}
EdgeSig& EdgeSig::operator|=(const EdgeSig& _sig)
{
	this->edgeBitSet |= _sig.edgeBitSet;
	return *this;
}
