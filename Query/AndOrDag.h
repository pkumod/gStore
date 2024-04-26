#ifndef _QUERY_ANDORDAG_H
#define _QUERY_ANDORDAG_H

#include "../Database/CSR.h"

class AndOrDag {
private:
    // Originally (in the rpq-view repository), q2idx & idx2q maps RPQ strings to and from 
    // AODC node indexes (subscripts), respectively.
    // Here they are modified to map predicate ids to and from AODC node indexes. The mappings of
    // other RPQs beyond single predicates (i.e., edge labels) are not needed.
    std::unordered_map<int, size_t> pred2idx;
    std::unordered_map<size_t, int> idx2pred;
public:
    AndOrDag(CSR *csr_) {}
    ~AndOrDag() {}
    void addWorkloadQuery(const std::string &q, size_t curFreq);    // Yue TODO
    int addQuery(const std::string &q); // Yue TODO
    void initAuxiliary() {} // Yue TODO

    // Anzhe TODO: when implementing the following member functions, suppose the AODC has been constructed.
    // If you need to test these functions when the functions above have not been implemented, you could
    // refer to AndOrDagTest.cpp in the rpq-view repository.
    void annotateLeafCostCard() {}
    void plan() {}
    void execute() {} // Consider executing the root node only.
};

#endif // _QUERY_ANDORDAG_H