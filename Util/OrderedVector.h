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
    static bool contain_sub_vec(const vector<int> &input, const vector<int> &small_vec);
    static void subtract(const vector<int> &input, const vector<int> &to_subtract, vector<int> &result);
    static void vec_set_union(vector<int> &vector_A, set<int> &set_B);


};


#endif //GSTORE_ORDEREDVECTOR_H
