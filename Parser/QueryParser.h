/*=============================================================================
# Filename: QueryParser.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-13
# Description:
=============================================================================*/

#ifndef QUERYPARSER_H_
#define QUERYPARSER_H_

#include "../Util/Util.h"
#include "../Query/QueryTree.h"
#include "SparqlParser.h"
#include "SparqlLexer.h"

class QueryParser {
  public:
  void SPARQLParse(const std::string& query, QueryTree& querytree);

  private:
  std::map<std::string, std::string> prefix_map;

  int printNode(pANTLR3_BASE_TREE node, int dep = 0);

  void parseWorkload(pANTLR3_BASE_TREE node, QueryTree& querytree);

  void parseQuery(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parsePrologue(pANTLR3_BASE_TREE node);
  void parsePrefix(pANTLR3_BASE_TREE node);
  void replacePrefix(std::string& str);
  void parseSelectClause(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parseSelectVar(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parseSelectAggregateFunction(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parseGroupPattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parsePattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parseOptionalOrMinus(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parseUnion(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parseFilter(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parseFilterTree(pANTLR3_BASE_TREE node, QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter);
  void parseVarInExpressionList(pANTLR3_BASE_TREE node, QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter);
  void parseBind(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parseGroupBy(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parseOrderBy(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parseString(pANTLR3_BASE_TREE node, std::string& str, int dep);

  void parseUpdate(pANTLR3_BASE_TREE node, QueryTree& querytree);
  void parseTripleTemplate(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern);
  void parseModify(pANTLR3_BASE_TREE node, QueryTree& querytree);
};

#endif /* QUERYPARSER_H_ */
