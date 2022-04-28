/*=============================================================================
# Filename:	QueryParser.h
# Author: Yue Pang
# Mail: michelle.py@pku.edu.cn
# Last Modified:	2021-08-03 16:58 CST
# Description: defines the class for parsing SPARQL queries based on ANTLR4
=============================================================================*/

#include <typeinfo>

#include "antlr4-runtime.h"
#include "SPARQL/SPARQLLexer.h"
#include "SPARQL/SPARQLParser.h"
#include "SPARQL/SPARQLBaseVisitor.h"
#include "../Query/QueryTree.h"
#include "../Util/Util.h"

/**
	Parser for SPARQL queries, inherited from SPARQLBaseVisitor, which is
	automatically generated from the SPARQL grammar by ANTLR. Implements
	visiting important grammatical units, extracts relevant information,
	and stores in QueryTree (pointed to by query_tree_ptr).
*/
class QueryParser: public SPARQLBaseVisitor
{
private:
	QueryTree *query_tree_ptr;
	std::map<std::string, std::string> prefix_map;
	bool firstVisitGroupGraphPatternSub;

public:
	QueryParser(): firstVisitGroupGraphPatternSub(true) {}
	QueryParser(QueryTree *qtp): firstVisitGroupGraphPatternSub(true), query_tree_ptr(qtp) {}
	void setQueryTree(QueryTree *qtp) { query_tree_ptr = qtp; }
	void SPARQLParse(const std::string &query);	// Overall driver function

	antlrcpp::Any visitQueryUnit(SPARQLParser::QueryUnitContext *ctx);
	antlrcpp::Any visitQuery(SPARQLParser::QueryContext *ctx);
	antlrcpp::Any visitSelectquery(SPARQLParser::SelectqueryContext *ctx);
	antlrcpp::Any visitAskquery(SPARQLParser::AskqueryContext *ctx);
	antlrcpp::Any visitWhereClause(SPARQLParser::WhereClauseContext *ctx, QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitLimitClause(SPARQLParser::LimitClauseContext *ctx);
	antlrcpp::Any visitOffsetClause(SPARQLParser::OffsetClauseContext *ctx);
	antlrcpp::Any visitPrefixDecl(SPARQLParser::PrefixDeclContext *ctx);
	antlrcpp::Any visitSelectClause(SPARQLParser::SelectClauseContext *ctx);
	antlrcpp::Any visitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitTriplesBlock(SPARQLParser::TriplesBlockContext *ctx, QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitFilter(SPARQLParser::FilterContext *ctx, QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitBind(SPARQLParser::BindContext *ctx, QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext *ctx, \
		QueryTree::GroupPattern &group_pattern);
	antlrcpp::Any visitGroupClause(SPARQLParser::GroupClauseContext *ctx);
	antlrcpp::Any visitOrderClause(SPARQLParser::OrderClauseContext *ctx);
	
	antlrcpp::Any visitInsertData(SPARQLParser::InsertDataContext *ctx);
	antlrcpp::Any visitDeleteData(SPARQLParser::DeleteDataContext *ctx);
	antlrcpp::Any visitDeleteWhere(SPARQLParser::DeleteWhereContext *ctx);
	antlrcpp::Any visitModify(SPARQLParser::ModifyContext *ctx);
	antlrcpp::Any visitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *ctx);

	void parseSelectAggregateFunction(SPARQLParser::ExpressionContext *expCtx, \
		SPARQLParser::VarContext *varCtx);
	void buildCompTree(antlr4::tree::ParseTree *root, int oper_pos, QueryTree::CompTreeNode &curr_node);
	void addTriple(std::string subject, std::string predicate, std::string object, bool kleene, \
		QueryTree::GroupPattern &group_pattern);
	void replacePrefix(std::string &str);
	std::string getNumeric(SPARQLParser::NumericLiteralContext *ctx);
	std::string getTextWithRange(antlr4::tree::ParseTree *ctx);

	void printNode(antlr4::ParserRuleContext *ctx, const char *nodeTypeName);
	void printTree(antlr4::tree::ParseTree *root, int dep);
	void printQueryTree();
};

/**
	Listener for errors during SPARQL query parsing, which throws a 
	corresponding exception when an error arises.
*/
class SPARQLErrorListener: public antlr4::BaseErrorListener
{
public:
	void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
		size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e);
};
