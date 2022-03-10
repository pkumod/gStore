/*=============================================================================
# Filename: OrderedVector.cpp
# Author: Linglin Yang
# Mail: fyulingi@gmail.com
=============================================================================*/

#include "OrderedVector.h"

using namespace std;


/**
 * @return if input contain small_vec, than return true, else return false;
 */
bool OrderedVector::contain_sub_vec(const vector<unsigned> &input, const vector<unsigned> &small_vec) {
    for(auto node : small_vec){
        if(find(input.begin(), input.end(), node) == input.end()){
            return false;
        }
    }
    return true;
}


/**
 * @param input ordered vector to subtract from
 * @param to_subtract ordered vector to subtract
 * @param result result ordered vector (input - to_subtract)
 */
void OrderedVector::subtract(const vector<unsigned> &input, const vector<unsigned> &to_subtract, vector<unsigned> &result) {
    for(auto x : input){
        if(find(to_subtract.begin(), to_subtract.end(), x) == to_subtract.end()){
            result.push_back(x);
        }
    }
}

/**
 * @param vector_A result ordered vector union with set_B
 */
void OrderedVector::vec_set_union(vector<unsigned> &vector_A, set<unsigned> &set_B) {
    for(auto x : set_B){
        if(find(vector_A.begin(), vector_A.end(), x) == vector_A.end()){
            vector_A.push_back(x);
        }
    }

    sort(vector_A.begin(), vector_A.end());
}