/*
 * QueryParser.h
 *
 *  Created on: 2015-4-11
 *      Author: cjq
 */

#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include "../Util/Util.h"
#include "../Query/QueryTree.h"
#include "SparqlParser.h"
#include "SparqlLexer.h"

class QueryParser{
private:
	std::map<std::string,std::string> _prefix_map;

	int printNode(pANTLR3_BASE_TREE node,int dep = 0);

	void parseTree(pANTLR3_BASE_TREE node, QueryTree& querytree);
	void parsePrologue(pANTLR3_BASE_TREE node);
	void parsePrefix(pANTLR3_BASE_TREE node);
	void replacePrefix(std::string& str);
	void parseSelectClause(pANTLR3_BASE_TREE node, QueryTree& querytree);
	void parseSelectVar(pANTLR3_BASE_TREE node, QueryTree& querytree);
	void parseGroupPattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern);
	void parsePattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern);
	void parseOptionalOrMinus(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern);
	void parseUnion(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern);
	void parseFilter(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern);
	void parseFilterTree(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern, QueryTree::GroupPattern::FilterTreeNode& filter);
	void parseVarInExpressionList(pANTLR3_BASE_TREE node, QueryTree::GroupPattern::FilterTreeNode& filter, unsigned int begin);
	void parseExistsGroupPattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& grouppattern, QueryTree::GroupPattern::FilterTreeNode& filter);
	void parseOrderBy(pANTLR3_BASE_TREE node, QueryTree& querytree);
	void parseString(pANTLR3_BASE_TREE node, std::string& str, int dep);


public:
	QueryParser();

	void sparqlParser(const std::string& query, QueryTree& querytree);
};

#endif /* QUERYPARSER_H_ */
