/*
 * SigEntry.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 *  Modified on: 2014-6-29
 *      Author: hanshuo
 */

#ifndef SIGENTRY_H_
#define SIGENTRY_H_

#include<iostream>
#include "Signature.h"
#include<sstream>
using namespace std;

class SigEntry{
private:
	EntitySig sig;
	int entity_id;
public:
	SigEntry();
	SigEntry(int _entity_id, EntityBitSet& _bitset);
	SigEntry(const SigEntry& _sig_entry);
	SigEntry(const EntitySig& sig, int _entity_id);
	const EntitySig& getEntitySig()const;
	int getEntityId()const;
	int getSigCount()const;
	SigEntry& operator=(const SigEntry _sig_entry);
	SigEntry& operator|=(const SigEntry _sig_entry);
	bool cover(const SigEntry& _sig_entry)const;
	bool cover(const EntitySig& _sig)const;
	int xEpsilen(const SigEntry& _sig_entry)const;
	std::string to_str()const;
};

#endif /* SIGENTRY_H_ */
