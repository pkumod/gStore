/*=============================================================================
# Filename: RDFParser.h
# Author: Yue Pang
# Mail: michelle.py@pku.edu.cn
# Last Modified:    2021-08-03 15:28 CST
# Description: defines the class for parsing RDF data during build based on 
RDF-3X's TurtleParser
=============================================================================*/

#ifndef gstore_parser_RDFParser
#define gstore_parser_RDFParser

#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "TurtleParser.h"

using namespace std;

/**  Parser for RDF data.
*/
class RDFParser
{
private:
    stringstream _sin;
    TurtleParser _TurtleParser;

public:
    static const int TRIPLE_NUM_PER_GROUP = 10 * 1000 * 1000;

    RDFParser();
    RDFParser(ifstream& _fin);
    int parseFile(TripleWithObjType* _triple_array, int& _triple_num, string _error_log="", int init_line=0);
    void parseString(string _str, TripleWithObjType* _triple_array, int& _triple_num);
};
#endif
