/*
 * DBparser.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef DBPARSER_H_
#define DBPARSER_H_

#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "../Query/SPARQLquery.h"
#include "SparqlParser.h"
#include "SparqlLexer.h"

using namespace std;

class DBparser {
  private:
  void printNode(pANTLR3_BASE_TREE node, int depth = 0);
  int parseNode(pANTLR3_BASE_TREE node, SPARQLquery& query, int depth = 0);
  int parsePrefix(pANTLR3_BASE_TREE node, std::pair<std::string, std::string>& prefixPair);
  int parseString(pANTLR3_BASE_TREE node, std::string& str, int depth = 0);
  int parseTriple(pANTLR3_BASE_TREE node, Triple& triple);
  int parseBasicQuery(pANTLR3_BASE_TREE node, BasicQuery& basicQuery);
  int parseVar(pANTLR3_BASE_TREE node, SPARQLquery& query);
  void replacePrefix(string& str);

  std::map<std::string, std::string> _prefix_map;

  public:
  /* how many triples at most will be parsed out when call rdfParser() once
	 * when -1, it means parse all triples in the file into the triples set(array)
	 * */
  static const int TRIPLE_NUM_PER_GROUP = 10 * 1000 * 1000;

  DBparser();

  /* input sparql query string and parse query into SPARQLquery
	 * the returned string is set for log when error happen */
  string sparqlParser(const string& _sparql, SPARQLquery& _sparql_query);

  /* file stream _fin points to rdfFile
	 * that was opened previously in Database::encodeRDF
	 * rdfParser() will be called many times until all triples in the rdfFile is parsed
	 * and after each call, a group of triples will be parsed into the vector;
	 * the returned string is set for log when error happen;
	 */
  string rdfParser(ifstream& _fin, Triple* _triple_array, int& _triple_num);

  private:
  /*
	 * used in readline of FILE, avoiding new memory each time
	 */
  static char* line_buf;
  static int buf_len;
};

#endif /* DBPARSER_H_ */
