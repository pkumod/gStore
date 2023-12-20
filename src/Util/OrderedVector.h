/*=============================================================================
# Filename: OrderedVector.h
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#ifndef GSTORE_ORDEREDVECTOR_H
#define GSTORE_ORDEREDVECTOR_H

#include <vector>
#include <set>
#include <algorithm>


using namespace std;

class OrderedVector {

public:
    static bool contain_sub_vec(const vector<unsigned> &input, const vector<unsigned> &small_vec);
    static void subtract(const vector<unsigned> &input, const vector<unsigned> &to_subtract, vector<unsigned> &result);
    static void vec_set_union(vector<unsigned> &vector_A, set<unsigned> &set_B);


};


#endif //GSTORE_ORDEREDVECTOR_H
