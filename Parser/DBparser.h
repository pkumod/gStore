/*
 * DBparser.h
 *
 *  Created on: 2015-4-11
 *      Author: cjq
 */

#ifndef DBPARSER_H_
#define DBPARSER_H_

#include<string>
#include<vector>
#include<map>
#include<fstream>
#include<stdio.h>
#include "../Query/SPARQLquery.h"
#include "SparqlParser.h"
#include "SparqlLexer.h"

class DBparser{
private:

	int printNode(pANTLR3_BASE_TREE node,int depth=0);

	void parseTree(pANTLR3_BASE_TREE node, SPARQLquery& query);
	void parsePrologue(pANTLR3_BASE_TREE node);
	void parsePrefix(pANTLR3_BASE_TREE node);
	void replacePrefix(std::string& str);
	void parseSelectClause(pANTLR3_BASE_TREE node, SPARQLquery& query);
	void parseSelectVar(pANTLR3_BASE_TREE node, SPARQLquery& query);
	void parseGroupPattern(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup);
	void parsePattern(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup);
	void parseOptional(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup);
	void parseUnion(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup);
	void parseFilter(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup);
	void parseFilterTree(pANTLR3_BASE_TREE node, SPARQLquery::FilterTree& filter);
	void parseString(pANTLR3_BASE_TREE node, std::string& str, int depth=0);

	void printquery(SPARQLquery& query);
	void printgrouppattern(SPARQLquery::PatternGroup &pg, int dep = 0);
	void printfilter(SPARQLquery::FilterTree &ft);

	void genQueryVec(SPARQLquery::PatternGroup &pg, SPARQLquery& query);
	std::map<std::string,std::string> _prefix_map;

public:
	DBparser();

	void sparqlParser(const std::string& _sparql, SPARQLquery& _sparql_query);
};

#endif /* DBPARSER_H_ */
