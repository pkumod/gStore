#ifndef _QUERY_ANDORDAG_H
#define _QUERY_ANDORDAG_H

#include "../Database/CSR.h"

class AndOrDag {
public:
    AndOrDag(CSR *csr_) {}
    ~AndOrDag() {}
    void addWorkloadQuery(const std::string &q, size_t curFreq);    // Yue TODO
    int addQuery(const std::string &q); // Yue TODO
    void initAuxiliary() {}
    void annotateLeafCostCard() {}
    void plan() {}
    void execute() {} // Execute the root node
};

#endif // _QUERY_ANDORDAG_H