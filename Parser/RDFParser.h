#ifndef gstore_parser_RDFParser
#define gstore_parser_RDFParser

#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "TurtleParser.h"

using namespace std;

class RDFParser {
  private:
  stringstream _sin;
  TurtleParser _TurtleParser;

  public:
  static const int TRIPLE_NUM_PER_GROUP = 10 * 1000 * 1000;

  //for parseString
  RDFParser()
      : _TurtleParser(_sin)
  {
  }

  //for parseFile
  RDFParser(ifstream& _fin)
      : _TurtleParser(_fin)
  {
  }

  string parseFile(TripleWithObjType* _triple_array, int& _triple_num);
  string parseFileSample(TripleWithObjType* _triple_array, int& _triple_num, int UPBOUND);
  string parseString(string _str, TripleWithObjType* _triple_array, int& _triple_num);
};
#endif
