// #include "../Util/Util.h"
#include <stdio.h>
#include <unordered_map>
#include <set>
#include <ctime>
#include <iostream>
#include <vector>
#include <map>
#include <string.h>
#include <fstream>
#include <queue>
#include <stack>
#include <cmath>

#ifndef _DATABASE_CSR_H
#define _DATABASE_CSR_H 

class CSR
{
public:
	unsigned pre_num;
	unsigned w;
	unsigned n,m;

	// A CSR for each predicate. For example, for the predicate p, use id2vid[p], vid2id[p], etc.
	std::vector<unsigned> *id2vid;	// Index (subscript) in offset_list to subject/object external ID
    std::map<unsigned, unsigned> *vid2id;	// Subject/object external ID to index (subscript) in offset_list
	std::vector<unsigned> *offset_list;	// Offsets in adjacency list
	// adjacency_list[p][offset_list[i]...offset_list[i+1]-1] are the external IDs of the neighbors of i
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

