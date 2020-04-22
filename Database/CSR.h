#include "../Util/Util.h"

#ifndef _DATABASE_CSR_H
#define _DATABASE_CSR_H 

class CSR
{
public:
	unsigned pre_num;
	unsigned w;

	// One for each predicate
	std::vector<unsigned> *id2vid;	// Index to subject/object ID
    std::map<unsigned, unsigned> *vid2id;	// Subject/object ID to index
	std::vector<unsigned> *offset_list;	// Offset in adjacency list
	std::vector<unsigned> *adjacency_list;
	// bool *valid;
	CSR();
	CSR(unsigned pnum);
	~CSR();
	void init(unsigned pnum);
	// void compress(unsigned w);

	void print();	// Feel free to modify this for testing
	long long sizeInBytes();
};

#endif //_DATABASE_CSR_H

