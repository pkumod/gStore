#include <typeinfo>

#include "antlr4-runtime.h"
#include "SPARQL/SPARQLLexer.h"
#include "SPARQL/SPARQLParser.h"
#include "SPARQL/SPARQLBaseVisitor.h"
#include "../Query/QueryTree.h"
#include "../Util/Util.h"

class QueryParser: public SPARQLBaseVisitor
{
private:
	QueryTree *query_tree_ptr;
	std::map<std::string, std::string> prefix_map;

public:
	QueryParser() {}
	QueryParser(QueryTree *qtp): query_tree_ptr(qtp) {}
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
	void buildCompTree(antlr4::tree::ParseTree *root, int oper_pos, QueryTree::CompTreeNode *curr_node);
	void buildFilterTree(antlr4::tree::ParseTree *root, \
		QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild *currChild, \
		QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, std::string tp);
	void addTriple(std::string subject, std::string predicate, std::string object, \
		QueryTree::GroupPattern &group_pattern);
	void replacePrefix(std::string &str);
	std::string getNumeric(SPARQLParser::NumericLiteralContext *ctx);
	std::string getTextWithRange(antlr4::tree::ParseTree *ctx);

	void printNode(antlr4::ParserRuleContext *ctx, const char *nodeTypeName);
	void printTree(antlr4::tree::ParseTree *root, int dep);
	void printQueryTree();
};

class SPARQLErrorListener: public antlr4::BaseErrorListener
{
public:
	void syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
		size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e);
};