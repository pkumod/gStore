#include "QueryParser.h"

using namespace std;

/**
	Throw a runtime error when lexical and syntactic errors are detected in the query.
*/
void SPARQLErrorListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
	size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e)
{
	throw runtime_error("line " + to_string(line) + ":" + to_string(charPositionInLine) + " " + msg);
}

/**
	Overall driver function for query parsing.

	@param query the query string.
*/
void QueryParser::SPARQLParse(const string &query)
{
	istringstream ifs(query);

	SPARQLErrorListener lstnr;

	antlr4::ANTLRInputStream input(ifs);
	SPARQLLexer lexer(&input);
	lexer.removeErrorListeners();
	lexer.addErrorListener(&lstnr);

	antlr4::CommonTokenStream tokens(&lexer);
	SPARQLParser parser(&tokens);
	parser.removeErrorListeners();
	parser.addErrorListener(&lstnr);

	SPARQLParser::EntryContext *tree = parser.entry();
	// printTree(tree, 0);
	visitEntry(tree);
}

/**
	Print the info of the parse tree node pointed to by ctx.

	@param ctx pointer to the requested parse tree node's context.
	@param nodeTypeName type of the parse tree node.
*/
void QueryParser::printNode(antlr4::ParserRuleContext *ctx, 
	const char *nodeTypeName)
{
	int dep = ctx->depth();

	// Print tabs according to node's depth in tree
	for (int i = 0; i < dep; i++)
		cout << '\t';

	// Print node type
	cout << "Type: " << nodeTypeName << ' ';

	// Print what text a node has matched
	cout << "Text: " << ctx->getText() << endl;

}

/**
	Print the sub-parse-tree rooted at root.

	@param root pointer to the root of the sub-parse-tree.
	@param dep depth of the sub-parse-tree's root.
*/
void QueryParser::printTree(antlr4::tree::ParseTree *root, int dep)
{
	// Print tabs according to node's depth in tree
	for (int i = 0; i < dep; i++)
		cout << '-';

	string nodeTypeName = typeid(*root).name();
	size_t n = root->children.size();
	cout << "Type: " << nodeTypeName << ' ';
	cout << "Text: " << root->getText() << ' ';
	cout << "#Children: " << n << endl;

	for (size_t i = 0; i < n; i++)
	{
		antlr4::tree::ParseTree *childNode = root->children[i];
		printTree(childNode, dep + 1);
	}
}

/**
	Print the QueryTree associated with this query.
*/
void QueryParser::printQueryTree()
{
	query_tree_ptr->print();
}

/**
	queryUnit : query ;
	Visit node queryUnit: recursively call visit on its only child node query.
	(Redundant, can be removed without affecting QueryParser's function)

	@param ctx pointer to queryUnit's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitQueryUnit(SPARQLParser::QueryUnitContext *ctx)
{
	// printNode(ctx, "queryUnit");

	visit(ctx->query());

	return antlrcpp::Any();
}

/**
	query : prologue( selectquery | constructquery | describequery | askquery )valuesClause ;
	Visit node query: recursively call visit on each of its children.
	(Redundant, can be removed without affecting QueryParser's function)

	@param ctx pointer to query's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitQuery(SPARQLParser::QueryContext *ctx)
{
	// printNode(ctx, "query");

	visit(ctx->prologue());

	// Only one of the following children is valid
	if (ctx->selectquery())
	{
		query_tree_ptr->setQueryForm(QueryTree::Select_Query);
		visit(ctx->selectquery());
	}
	if (ctx->constructquery())	// Not supported?
		visit(ctx->constructquery());
	if (ctx->describequery())	// Not supported?
		visit(ctx->describequery());
	if (ctx->askquery())
	{
		query_tree_ptr->setQueryForm(QueryTree::Ask_Query);
		visit(ctx->askquery());
	}

	visit(ctx->valuesClause());

	return antlrcpp::Any();
}

/**
	selectquery : selectClause datasetClause* whereClause solutionModifier ;
	Visit node selectquery: recursively call visit on each of its children, except
	datasetClause (currently not supported). When visiting whereClause, pass the 
	reference to QueryTree's outermost group graph pattern for it to be filled.

	@param ctx pointer to selectquery's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitSelectquery(SPARQLParser::SelectqueryContext *ctx)
{
	visit(ctx->selectClause());

	// datasetClause not supported

	// Call explictly to pass group_pattern as a parameter
	visitWhereClause(ctx->whereClause(), query_tree_ptr->getGroupPattern());

	visit(ctx->solutionModifier());

	return antlrcpp::Any();
}

/**
	askquery : K_ASK datasetClause* whereClause solutionModifier ;
	Visit node askquery: recursively call visit on each of its children, except
	datasetClause (currently not supported). When visiting whereClause, pass the 
	reference to QueryTree's outermost group graph pattern for it to be filled.
	Also set the query form as ASK, and set the projection asterisk (equivalent
	to SELECT * without returning actual results).

	@param ctx pointer to askquery's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitAskquery(SPARQLParser::AskqueryContext *ctx)
{
	query_tree_ptr->setQueryForm(QueryTree::Ask_Query);
	query_tree_ptr->setProjectionAsterisk();

	// datasetClause not supported

	// Call explictly to pass group_pattern as a parameter
	visitWhereClause(ctx->whereClause(), query_tree_ptr->getGroupPattern());

	visit(ctx->solutionModifier());

	return antlrcpp::Any();
}

/**
	whereClause : K_WHERE? groupGraphPattern ;
	Visit node whereClause: recursively call visit on its child groupGraphPattern, 
	passing on the reference to QueryTree's outermost group graph pattern for it to 
	be filled.

	@param ctx pointer to whereClause's context.
	@param group_pattern the reference to QueryTree's outermost group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitWhereClause(SPARQLParser::WhereClauseContext *ctx, QueryTree::GroupPattern &group_pattern)
{
	// Call explictly to pass group_pattern as a parameter
	visitGroupGraphPattern(ctx->groupGraphPattern(), group_pattern);

	return antlrcpp::Any();
}

/**
	limitClause : K_LIMIT INTEGER ;
	Visit node limitClause: call setLimit with parameter as limitClause's child INTEGER.

	@param ctx pointer to limitClause's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitLimitClause(SPARQLParser::LimitClauseContext *ctx)
{
	query_tree_ptr->setLimit(stoi(getTextWithRange(ctx->children[1])));

	return antlrcpp::Any();
}

/**
	offsetClause : K_OFFSET INTEGER ;
	Visit node offsetClause: call setOffset with parameter as offsetClause's child INTEGER.

	@param ctx pointer to offsetClause's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitOffsetClause(SPARQLParser::OffsetClauseContext *ctx)
{
	query_tree_ptr->setOffset(stoi(getTextWithRange(ctx->children[1])));

	return antlrcpp::Any();
}

/**
	prefixDecl : K_PREFIX PNAME_NS IRIREF ;
	Visit node prefixDecl: construct the mapping between the prefix key (PNAME_NS) 
	and value (IRIREF) in prefix_map.

	@param ctx pointer to prefixDecl's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitPrefixDecl(SPARQLParser::PrefixDeclContext *ctx)
{
	string key, value;

	key = ctx->PNAME_NS()->getText();
	value = ctx->IRIREF()->getText();

	prefix_map[key] = value;

	return antlrcpp::Any();
}

/**
	selectClause : K_SELECT ( K_DISTINCT | K_REDUCED )? ( ( var | expressionAsVar )+ | '*' ) ;
	Visit node selectClause: set corresponding projection modifier if DISTINCT (REDUCED is 
	currently not supported); gather the variables to project.

	@param ctx pointer to selectClause's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitSelectClause(SPARQLParser::SelectClauseContext *ctx)
{
	// printNode(ctx, "selectClause");

	if (ctx->children[1]->children.size() == 0)
	{
		string tmp = ctx->children[1]->getText();
		transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
		if (tmp == "DISTINCT")
			query_tree_ptr->setProjectionModifier(QueryTree::Modifier_Distinct);
		else if (tmp == "REDUCED")
			throw runtime_error("[ERROR]	REDUCED is currently not supported.");
		else if (tmp == "*")
			query_tree_ptr->setProjectionAsterisk();
	}

	if (ctx->children.size() == 3 && ctx->children[2]->children.size() == 0)	// '*'
		query_tree_ptr->setProjectionAsterisk();
	else
	{
		// Order of selected elements does not matter
		for (auto var : ctx->var())
		{
			query_tree_ptr->addProjectionVar();
			QueryTree::ProjectionVar &proj_var = query_tree_ptr->getLastProjectionVar();
			proj_var.aggregate_type = QueryTree::ProjectionVar::None_type;
			proj_var.var = var->getText();
		}
		for (auto expressionAsVar : ctx->expressionAsVar())
			parseSelectAggregateFunction(expressionAsVar->expression(), \
				expressionAsVar->var());
	}

	return antlrcpp::Any();
}

/**
	Deal with expressionAsVar in selectClause: get the type of built-in call, its arguments, 
	and the variable following AS.

	@param expCtx pointer to expression's context (in expressionAsVar).
	@param varCtx pointer to var's context (in expressionAsVar).
	@return a dummy antlrcpp::Any object.
*/
void QueryParser::parseSelectAggregateFunction(SPARQLParser::ExpressionContext *expCtx, \
	SPARQLParser::VarContext *varCtx)
{
	// Sanity checks
	SPARQLParser::BuiltInCallContext *bicCtx = expCtx->conditionalOrexpression()-> \
		conditionalAndexpression(0)->valueLogical(0)->relationalexpression()-> \
		numericexpression(0)->additiveexpression()->multiplicativeexpression(0)-> \
		unaryexpression(0)->primaryexpression()->builtInCall();
	// if (!bicCtx)
	// 	throw runtime_error("[ERROR]	Currently only support selecting variables, "
	// 		"the aggregate function COUNT, and path-associated built-in calls");
	if (bicCtx)
	{	
		antlr4::tree::ParseTree *curr = expCtx;
		for (int i = 0; i < 10; i++)
		{
			// Make sure only one children along the way
			if (curr->children.size() > 1)
				throw runtime_error("[ERROR] Currently only support selecting variables; "
					"the aggregate functions COUNT, MIN, MAX, SUM, AVG; "
					"the built-in call CONTAINS; and path-associated built-in calls");
			curr = curr->children[0];
		}
		SPARQLParser::AggregateContext *aggCtx = bicCtx->aggregate();
		if (aggCtx)
		{
			string tmp = aggCtx->children[0]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp != "COUNT" && tmp != "MIN" && tmp != "MAX" && tmp != "AVG" && tmp != "SUM")
				throw runtime_error("[ERROR] The supported aggregate function now is COUNT, MIN, MAX, AVG, SUM only");

			query_tree_ptr->addProjectionVar();
			QueryTree::ProjectionVar &proj_var = query_tree_ptr->getLastProjectionVar();
			if (tmp == "COUNT")
				proj_var.aggregate_type = QueryTree::ProjectionVar::Count_type;
			else if (tmp == "MIN")
				proj_var.aggregate_type = QueryTree::ProjectionVar::Min_type;
			else if (tmp == "MAX")
				proj_var.aggregate_type = QueryTree::ProjectionVar::Max_type;
			else if (tmp == "AVG")
				proj_var.aggregate_type = QueryTree::ProjectionVar::Avg_type;
			else if (tmp == "SUM")
				proj_var.aggregate_type = QueryTree::ProjectionVar::Sum_type;
			proj_var.aggregate_var = aggCtx->expression()->getText();	// Error would have been dealt with
			tmp = aggCtx->children[2]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			proj_var.distinct = aggCtx->children[2]->children.size() == 0 && tmp == "DISTINCT";
			proj_var.var = varCtx->getText();
		}
		else
		{
			string tmp = bicCtx->children[0]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp == "SIMPLECYCLEPATH" || tmp == "SIMPLECYCLEBOOLEAN"
				|| tmp == "CYCLEPATH" || tmp == "CYCLEBOOLEAN"
				|| tmp == "SHORTESTPATH" || tmp == "SHORTESTPATHLEN"
				|| tmp == "KHOPREACHABLE" || tmp == "KHOPENUMERATE")	// Path calls
			{
				query_tree_ptr->addProjectionVar();
				QueryTree::ProjectionVar &proj_var = query_tree_ptr->getLastProjectionVar();
				if (tmp == "SIMPLECYCLEPATH")
					proj_var.aggregate_type = QueryTree::ProjectionVar::simpleCyclePath_type;
				else if (tmp == "SIMPLECYCLEBOOLEAN")
					proj_var.aggregate_type = QueryTree::ProjectionVar::simpleCycleBoolean_type;
				else if (tmp == "CYCLEPATH")
					proj_var.aggregate_type = QueryTree::ProjectionVar::cyclePath_type;
				else if (tmp == "CYCLEBOOLEAN")
					proj_var.aggregate_type = QueryTree::ProjectionVar::cycleBoolean_type;
				else if (tmp == "SHORTESTPATH")
					proj_var.aggregate_type = QueryTree::ProjectionVar::shortestPath_type;
				else if (tmp == "SHORTESTPATHLEN")
					proj_var.aggregate_type = QueryTree::ProjectionVar::shortestPathLen_type;
				else if (tmp == "KHOPREACHABLE")
					proj_var.aggregate_type = QueryTree::ProjectionVar::kHopReachable_type;
				else if (tmp == "KHOPENUMERATE")
					proj_var.aggregate_type = QueryTree::ProjectionVar::kHopEnumerate_type;

				proj_var.path_args.src = bicCtx->varOrIri(0)->getText();
				replacePrefix(proj_var.path_args.src);
				proj_var.path_args.dst = bicCtx->varOrIri(1)->getText();
				replacePrefix(proj_var.path_args.dst);
				auto predSet = bicCtx->predSet()->iri();
				for (auto pred : predSet)
				{
					string prefixedPred = pred->getText();
					replacePrefix(prefixedPred);
					proj_var.path_args.pred_set.push_back(prefixedPred);
				}

				if (tmp == "KHOPREACHABLE" || tmp == "KHOPENUMERATE")
				{
					if (bicCtx->num_integer())
						proj_var.path_args.k = stoi(getTextWithRange(bicCtx->num_integer()));
					else if (bicCtx->integer_positive())
						proj_var.path_args.k = stoi(getTextWithRange(bicCtx->integer_positive()));
					else if (bicCtx->integer_negative())
						proj_var.path_args.k = stoi(getTextWithRange(bicCtx->integer_negative()));
					
					if (bicCtx->numericLiteral())
						proj_var.path_args.confidence = stof(bicCtx->numericLiteral()->getText());
					else
						proj_var.path_args.confidence = 1;
				}
				if (bicCtx->booleanLiteral()->getText() == "true")
					proj_var.path_args.directed = true;
				else
					proj_var.path_args.directed = false;

				proj_var.var = varCtx->getText();
				
			}
			else if (tmp == "CONTAINS")	// Original built-in calls, may add others later
			{
				query_tree_ptr->addProjectionVar();
				QueryTree::ProjectionVar &proj_var = query_tree_ptr->getLastProjectionVar();
				proj_var.aggregate_type = QueryTree::ProjectionVar::Contains_type;
				proj_var.builtin_args.push_back(bicCtx->expression(0)->getText());
				proj_var.builtin_args.push_back(bicCtx->expression(1)->getText());
				proj_var.var = varCtx->getText();
			}
			else
				throw runtime_error("[ERROR] Currently only support selecting variables; "
					"the aggregate functions COUNT, MIN, MAX, SUM, AVG; "
					"the built-in call CONTAINS; and path-associated built-in calls");
				
			
		}
	}
	else 	// For multi-layer computation, only consider vars, literals, and bracketted expressions for now
	{
		query_tree_ptr->addProjectionVar();
		QueryTree::ProjectionVar &proj_var = query_tree_ptr->getLastProjectionVar();
		proj_var.aggregate_type = QueryTree::ProjectionVar::CompTree_type;
		proj_var.var = varCtx->getText();
		proj_var.comp_tree_root = new QueryTree::CompTreeNode;
		buildCompTree(expCtx, -1, proj_var.comp_tree_root);
	}
}

void QueryParser::buildCompTree(antlr4::tree::ParseTree *root, int oper_pos, QueryTree::CompTreeNode *curr_node)
{
	cout << root->getText() << endl;
	cout << "#children = " << root->children.size() << endl;

	if (root->children.size() == 1)
	{
		if (((SPARQLParser::PrimaryexpressionContext *)root)->rDFLiteral())
		{
			curr_node->oprt = "";
			curr_node->lchild = NULL;
			curr_node->rchild = NULL;
			curr_node->val = root->getText();
		}
		else if (((SPARQLParser::PrimaryexpressionContext *)root)->numericLiteral())
		{
			curr_node->oprt = "";
			curr_node->lchild = NULL;
			curr_node->rchild = NULL;
			auto numericLiteral = ((SPARQLParser::PrimaryexpressionContext *)root)->numericLiteral();
			curr_node->val = getNumeric(numericLiteral);
		}
		else if (((SPARQLParser::PrimaryexpressionContext *)root)->booleanLiteral())
		{
			curr_node->oprt = "";
			curr_node->lchild = NULL;
			curr_node->rchild = NULL;
			curr_node->val = "\"" + root->getText() + "\"" + "^^<http://www.w3.org/2001/XMLSchema#boolean>";
		}
		else if (((SPARQLParser::PrimaryexpressionContext *)root)->var())
		{
			curr_node->oprt = "";
			curr_node->lchild = NULL;
			curr_node->rchild = NULL;
			curr_node->val = root->getText();
		}
		else
			buildCompTree(root->children[0], -1, curr_node);
	}
	else if (root->children.size() == 2)
	{
		if (root->children[0]->getText()[0] != '!' \
			&& root->children[0]->getText()[0] != '+' \
			&& root->children[0]->getText()[0] != '-')
			throw runtime_error("[ERROR]	Unary operator not supported");
		curr_node->oprt = root->children[0]->getText();
		curr_node->lchild = new QueryTree::CompTreeNode;
		curr_node->rchild = NULL;
		curr_node->val = "";
		buildCompTree(root->children[1], -1, curr_node->lchild);
	}
	else if (root->children.size() % 2 == 1)	// >= 3, odd #children
	{
		if (root->children[0]->getText() == "(")
			buildCompTree(root->children[1], -1, curr_node);
		else
		{
			int rightmostOprtPos = (root->children.size() - 1) / 2;
			// if (oper_pos == -1)
			// 	oper_pos = 1;
			if (oper_pos < rightmostOprtPos)
			{
				int new_oper_pos = oper_pos + 2;
				curr_node->oprt = root->children[oper_pos + 2]->getText();
				curr_node->lchild = new QueryTree::CompTreeNode;
				curr_node->rchild = new QueryTree::CompTreeNode;
				curr_node->val = "";
				buildCompTree(root->children[oper_pos + 1], -1, curr_node->lchild);
				buildCompTree(root, oper_pos + 2, curr_node->rchild);
			}
			else 	// oper_pos == rightmostOprtPos, the last operator on this level has been handled
				buildCompTree(root->children[oper_pos + 1], -1, curr_node);
		}
	}
	else
		throw runtime_error("[ERROR]	Computation type not supported (an even #children > 2)");
}

/**
	groupGraphPattern : '{' ( subSelect | groupGraphPatternSub ) '}' ;
	Visit node groupGraphPattern: recursively visit its child groupGraphPatternSub (subSelect
	is currently not supported), passing the reference to a group graph pattern as parameter.

	@param ctx pointer to groupGraphPattern's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	// subSelect not supported

	visitGroupGraphPatternSub(ctx->groupGraphPatternSub(), group_pattern);

	return antlrcpp::Any(); 
}

/**
	groupGraphPatternSub : triplesBlock? graphPatternTriplesBlock* ;
	graphPatternTriplesBlock : graphPatternNotTriples '.'? triplesBlock? ;
	Visit node groupGraphPattern: recursively visit its descendants triplesBlock and 
	graphPatternTriplesBlock in order, passing the reference to a group graph pattern 
	as parameter.

	@param ctx pointer to groupGraphPatternSub's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitGroupGraphPatternSub(SPARQLParser::GroupGraphPatternSubContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	if (ctx->triplesBlock())
		visitTriplesBlock(ctx->triplesBlock(), group_pattern);
	for (auto graphPatternTriplesBlock : ctx->graphPatternTriplesBlock())
	{
		visitGraphPatternNotTriples(graphPatternTriplesBlock->graphPatternNotTriples(), \
			group_pattern);
		if (graphPatternTriplesBlock->triplesBlock())
			visitTriplesBlock(graphPatternTriplesBlock->triplesBlock(), group_pattern);
	}

	return antlrcpp::Any();
}

/**
	triplesBlock : triplesSameSubjectpath ( '.' triplesBlock? )? ;
	Visit node triplesBlock: recursively visit its children triplesSameSubjectpath and 
	triplesBlock in order, passing the reference to a group graph pattern as parameter.

	@param ctx pointer to triplesBlock's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitTriplesBlock(SPARQLParser::TriplesBlockContext *ctx, QueryTree::GroupPattern &group_pattern)
{
	visitTriplesSameSubjectpath(ctx->triplesSameSubjectpath(), group_pattern);
	if (ctx->triplesBlock())
		visitTriplesBlock(ctx->triplesBlock(), group_pattern);

	return antlrcpp::Any();
}

/**
	graphPatternNotTriples : groupOrUnionGraphPattern | optionalGraphPattern | 
	minusGraphPattern | graphGraphPattern | serviceGraphPattern | filter | bind | inlineData ;
	Visit node graphPatternNotTriples: recursively visit its child, passing the reference to 
	a group graph pattern as parameter.

	@param ctx pointer to graphPatternNotTriples's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitGraphPatternNotTriples(SPARQLParser::GraphPatternNotTriplesContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	if (ctx->groupOrUnionGraphPattern())
		visitGroupOrUnionGraphPattern(ctx->groupOrUnionGraphPattern(), group_pattern);
	else if (ctx->optionalGraphPattern())
		visitOptionalGraphPattern(ctx->optionalGraphPattern(), group_pattern);
	else if (ctx->minusGraphPattern())
		visitMinusGraphPattern(ctx->minusGraphPattern(), group_pattern);
	// graphGraphPattern not supported
	// serviceGraphPattern not supported
	else if (ctx->filter())
		visitFilter(ctx->filter(), group_pattern);
	else if (ctx->bind())
		visitBind(ctx->bind(), group_pattern);

	return antlrcpp::Any();
}

/**
	groupOrUnionGraphPattern : groupGraphPattern ( K_UNION groupGraphPattern )* ;
	Visit node groupOrUnionGraphPattern: if there is only one child, it is an individual 
	nested group graph pattern; if there are multiple children, they are connected by UNION.
	Add corresponding components to group_pattern, and fill them by recursively visiting
	the children.

	@param ctx pointer to groupOrUnionGraphPattern's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	if (ctx->children.size() == 1)
	{
		group_pattern.addOneGroup();
		visitGroupGraphPattern(ctx->groupGraphPattern(0), group_pattern.getLastGroup());
	}
	else
	{
		group_pattern.addOneGroupUnion();
		for (auto groupGraphPattern : ctx->groupGraphPattern())
		{
			group_pattern.addOneUnion();
			visitGroupGraphPattern(groupGraphPattern, group_pattern.getLastUnion());
		}
	}

	return antlrcpp::Any();
}

/**
	optionalGraphPattern : K_OPTIONAL groupGraphPattern ;
	Visit node optionalGraphPattern: Add an OPTIONAL to group_pattern, and fill its following
	group graph pattern by recursively visiting the child groupGraphPattern.

	@param ctx pointer to optionalGraphPattern's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	group_pattern.addOneOptional(QueryTree::GroupPattern::SubGroupPattern::Optional_type);
	visitGroupGraphPattern(ctx->groupGraphPattern(), group_pattern.getLastOptional());

	return antlrcpp::Any();
}

/**
	minusGraphPattern : K_MINUS groupGraphPattern ;
	Visit node minusGraphPattern: Add a MINUS to group_pattern, and fill its following
	group graph pattern by recursively visiting the child groupGraphPattern.

	@param ctx pointer to minusGraphPattern's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	group_pattern.addOneOptional(QueryTree::GroupPattern::SubGroupPattern::Minus_type);
	visitGroupGraphPattern(ctx->groupGraphPattern(), group_pattern.getLastOptional());

	return antlrcpp::Any();
}

/**
	filter : K_FILTER constraint ;
	Visit node filter: Add a FILTER to group_pattern, and construct the filter tree by
	calling buildFilterTree.

	@param ctx pointer to filter's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitFilter(SPARQLParser::FilterContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	if (ctx->constraint()->functionCall())
		throw runtime_error("[ERROR]	Function call currently not supported in filter.");

	group_pattern.addOneFilter();

	if (ctx->constraint()->brackettedexpression())
		buildFilterTree(ctx->constraint()->brackettedexpression()->expression()->conditionalOrexpression(), \
			NULL, group_pattern.getLastFilter().root, "conditionalOrexpression");
	else if (ctx->constraint()->builtInCall())
		buildFilterTree(ctx->constraint()->builtInCall(), NULL, \
			group_pattern.getLastFilter().root, "builtInCall");

	return antlrcpp::Any();
}

/**
	Construct FilterTree by DFS on FILTER's constraint.

	@param root pointer to the root of the sub-constraint-tree.
	@param currChild pointer to the current FilterTreeChild (NULL if at the root of 
	the FilterTree; the parameter filter is needed because of this).
	@param filter reference to the current FilterTreeNode (attribute of the current 
	FilterTreeChild except when at the root of the FilterTree).
	@param tp type of the current level of constraint (see grammar).
*/
void QueryParser::buildFilterTree(antlr4::tree::ParseTree *root, \
	QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild *currChild, \
	QueryTree::GroupPattern::FilterTree::FilterTreeNode &filter, string tp)
{
	string tmp;

	// Passing argument like so lose will type info; therefore needs string tp
	if (tp == "conditionalOrexpression")
	{
		if (root->children.size() == 1)
			buildFilterTree(((SPARQLParser::ConditionalOrexpressionContext *)root)->conditionalAndexpression(0), \
				currChild, filter, "conditionalAndexpression");
		else
		{
			filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Or_type;
			if (currChild)
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;

			int numChild = 0;
			for (auto conditionalAndexpression : \
				((SPARQLParser::ConditionalOrexpressionContext *)root)->conditionalAndexpression())
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				buildFilterTree(conditionalAndexpression, &(filter.child[numChild]), \
					filter.child[numChild].node, "conditionalAndexpression");
				numChild++;
			}
		}
	}
	else if (tp == "conditionalAndexpression")
	{
		if (root->children.size() == 1)
			buildFilterTree(((SPARQLParser::ConditionalAndexpressionContext *)root) \
				->valueLogical(0)->relationalexpression(), currChild, filter, "relationalexpression");
		else
		{
			filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::And_type;
			if (currChild)
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;

			int numChild = 0;
			for (auto valueLogical : \
				((SPARQLParser::ConditionalAndexpressionContext *)root)->valueLogical())
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				buildFilterTree(valueLogical->relationalexpression(), &(filter.child[numChild]), \
					filter.child[numChild].node, "relationalexpression");
				numChild++;
			}
		}
	}
	else if (tp == "relationalexpression")
	{
		if (root->children.size() == 1)
			buildFilterTree(((SPARQLParser::RelationalexpressionContext *)root)->numericexpression(0), \
				currChild, filter, "numericexpression");
		else
		{
			string op = root->children[1]->getText();
			transform(op.begin(), op.end(), op.begin(), ::toupper);
			if (op == "=")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Equal_type;
			else if (op == "!=")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::NotEqual_type;
			else if (op == "<")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Less_type;
			else if (op == ">")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Greater_type;
			else if (op == "<=")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::LessOrEqual_type;
			else if (op == ">=")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::GreaterOrEqual_type;
			else if (op == "IN")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_in_type;
			else if (op == "NOT")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Not_type;

			if (currChild)
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;

			tmp = root->children[1]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp == "NOT")	// NOT IN
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				filter.child[0].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
				filter.child[0].node.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_in_type;
				
				filter = filter.child[0].node;
				int numChild = 0;
				for (auto expression : \
					((SPARQLParser::RelationalexpressionContext *)root)->expressionList()->expression())
				{
					filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
					buildFilterTree(expression->conditionalOrexpression(), &(filter.child[numChild]), \
						filter.child[numChild].node, "conditionalOrexpression");
					numChild++;
				}
			}
			else if (tmp == "IN")	// IN
			{
				int numChild = 0;
				for (auto expression : \
					((SPARQLParser::RelationalexpressionContext *)root)->expressionList()->expression())
				{
					filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
					buildFilterTree(expression->conditionalOrexpression(), &(filter.child[numChild]), \
						filter.child[numChild].node, "conditionalOrexpression");
					numChild++;
				}
			}
			else
			{
				int numChild = 0;
				for (auto numericexpression : \
					((SPARQLParser::RelationalexpressionContext *)root)->numericexpression())
				{
					filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
					buildFilterTree(numericexpression, &(filter.child[numChild]), \
						filter.child[numChild].node, "numericexpression");
					numChild++;
				}
			}
		}
	}
	else if (tp == "numericexpression")
	{
		if (((SPARQLParser::NumericexpressionContext *)root)->additiveexpression()->children.size() > 1
			|| ((SPARQLParser::NumericexpressionContext *)root)->additiveexpression() \
			->multiplicativeexpression(0)->children.size() > 1)
			throw runtime_error("[ERROR]	Filter currently does not support numeric computation.");
		auto unaryexpression = ((SPARQLParser::NumericexpressionContext *)root)-> \
			additiveexpression()->multiplicativeexpression(0)->unaryexpression(0);
		buildFilterTree(unaryexpression, currChild, filter, "unaryexpression");
	}
	else if (tp == "unaryexpression")
	{
		if (root->children.size() == 2)
		{
			if (root->children[0]->getText() == "!")
			{
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Not_type;
				if (currChild)
					currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				buildFilterTree(((SPARQLParser::UnaryexpressionContext *)root)->primaryexpression(), \
					&(filter.child[0]), filter.child[0].node, "primaryexpression");
			}
			else
				throw runtime_error("[ERROR]	Filter currently does not support numeric computation.");
		}
		else
			buildFilterTree(((SPARQLParser::UnaryexpressionContext *)root)->primaryexpression(), \
				currChild, filter, "primaryexpression");
	}
	else if (tp == "primaryexpression")
	{
		if (((SPARQLParser::PrimaryexpressionContext *)root)->brackettedexpression())
			buildFilterTree(((SPARQLParser::PrimaryexpressionContext *)root)->brackettedexpression()-> \
				expression()->conditionalOrexpression(), currChild, filter, "conditionalOrexpression");
		else if (((SPARQLParser::PrimaryexpressionContext *)root)->builtInCall())
			buildFilterTree(((SPARQLParser::PrimaryexpressionContext *)root)->builtInCall(), \
				currChild, filter, "builtInCall");
		else if (((SPARQLParser::PrimaryexpressionContext *)root)->iriOrFunction())
			throw runtime_error("[ERROR]	Filter currently does not support custom function call.");
		else	// rDFLiteral | numericLiteral | booleanLiteral | var
		{
			if (currChild)	// This SHOULD hold true, unless someone only writes one var or literal 
				// inside brackets to constitute brackettedexpression, which is meaningless
			{
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
				
				if (((SPARQLParser::PrimaryexpressionContext *)root)->rDFLiteral())
					currChild->str = getTextWithRange(root);
				else if (((SPARQLParser::PrimaryexpressionContext *)root)->numericLiteral())
				{
					auto numericLiteral = ((SPARQLParser::PrimaryexpressionContext *)root)->numericLiteral();
					currChild->str = getNumeric(numericLiteral);
				}
				else if (((SPARQLParser::PrimaryexpressionContext *)root)->booleanLiteral())
					currChild->str = "\"" + root->getText() + "\"" + "^^<http://www.w3.org/2001/XMLSchema#boolean>";
				else if (((SPARQLParser::PrimaryexpressionContext *)root)->var())
					currChild->str = root->getText();
				
				replacePrefix(currChild->str);
			}
		}
	}
	else if (tp == "builtInCall")
	{
		if (((SPARQLParser::BuiltInCallContext *)root)->regexexpression())
		{
			filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_regex_type;
			if (currChild)
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
			
			int numChild = 0;
			for (auto expression : \
				((SPARQLParser::BuiltInCallContext *)root)->regexexpression()->expression())
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				buildFilterTree(expression->conditionalOrexpression(), &(filter.child[numChild]), \
					filter.child[numChild].node, "conditionalOrexpression");
				numChild++;
			}
		}
		else if (root->children[0]->children.size() == 0)
		{
			string funcName = root->children[0]->getText();
			transform(funcName.begin(), funcName.end(), funcName.begin(), ::toupper);
			if (funcName == "STR")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_str_type;
			else if (funcName == "ISIRI")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isiri_type;
			else if (funcName == "ISURI")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isuri_type;
			else if (funcName == "ISLITERAL")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isliteral_type;
			else if (funcName == "ISNUMERIC")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isnumeric_type;
			else if (funcName == "LANG")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_lang_type;
			else if (funcName == "LANGMATCHES")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_langmatches_type;
			else if (funcName == "BOUND")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_bound_type;
			else if (funcName == "SIMPLECYCLEBOOLEAN")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_simpleCycle_type;
			else if (funcName == "CYCLEBOOLEAN")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_cycle_type;
			else if (funcName == "SHORTESTPATHLEN")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_sp_type;
			else if (funcName == "KHOPREACHABLE")
				filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_khop_type;
			else
				throw runtime_error("[ERROR] Filter currently does not support this built-in call.");

			if (currChild)
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;

			tmp = root->children[0]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp == "BOUND")
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				filter.child[0].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
				filter.child[0].str = ((SPARQLParser::BuiltInCallContext *)root)->var()->getText();
				replacePrefix(filter.child[0].str);
			}
			else if (tmp == "SIMPLECYCLEBOOLEAN" || tmp == "CYCLEBOOLEAN" \
				|| tmp == "SHORTESTPATHLEN" || tmp == "KHOPREACHABLE")
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				filter.child[0].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
				filter.child[0].path_args.src = ((SPARQLParser::BuiltInCallContext *)root)->varOrIri(0)->getText();
				replacePrefix(filter.child[0].path_args.src);
				filter.child[0].path_args.dst = ((SPARQLParser::BuiltInCallContext *)root)->varOrIri(1)->getText();
				replacePrefix(filter.child[0].path_args.dst);
				auto predSet = ((SPARQLParser::BuiltInCallContext *)root)->predSet()->iri();
				for (auto pred : predSet)
				{
					string prefixedPred = pred->getText();
					replacePrefix(prefixedPred);
					filter.child[0].path_args.pred_set.push_back(prefixedPred);
				}

				if (tmp == "KHOPREACHABLE")
				{
					filter.child[0].path_args.k = \
						stoi(((SPARQLParser::BuiltInCallContext *)root)->num_integer()->getText());
					filter.child[0].path_args.confidence = \
						stof(((SPARQLParser::BuiltInCallContext *)root)->numericLiteral()->getText());
				}
				if (((SPARQLParser::BuiltInCallContext *)root)->booleanLiteral()->getText() == "true")
					filter.child[0].path_args.directed = true;
				else
					filter.child[0].path_args.directed = false;
			}
			else
			{
				int numChild = 0;
				for (auto expression : ((SPARQLParser::BuiltInCallContext *)root)->expression())
				{
					filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
					buildFilterTree(expression->conditionalOrexpression(), &(filter.child[numChild]), \
						filter.child[numChild].node, "conditionalOrexpression");
					numChild++;
				}
			}
		}
		else
			throw runtime_error("[ERROR]	Filter currently does not support this built-in call.");
	}
	else
		throw runtime_error("[ERROR]	Unaccounted for route when parsing filter.");
}

/**
	bind : K_BIND '(' expression K_AS var ')' ;
	Visit node bind: Add a BIND to group_pattern, and fill in the string and variable 
	to be bound.

	@param ctx pointer to bind's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitBind(SPARQLParser::BindContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	SPARQLParser::RDFLiteralContext *rdflCtx = ctx->expression()->conditionalOrexpression()-> \
		conditionalAndexpression(0)->valueLogical(0)->relationalexpression()-> \
		numericexpression(0)->additiveexpression()->multiplicativeexpression(0)-> \
		unaryexpression(0)->primaryexpression()->rDFLiteral();
	if (!rdflCtx)
		throw runtime_error("[ERROR]	Currently BIND only supports assigning a string to a var.");
	antlr4::tree::ParseTree *curr = ctx;
	for (int i = 0; i < 10; i++)
	{
		// Make sure only one children along the way
		if (curr->children.size() > 1)
			throw runtime_error("[ERROR]	Currently BIND only supports assigning a string to a var.");
		curr = curr->children[0];
	}

	string str, var;
	str = rdflCtx->string()->getText();
	var = ctx->var()->getText();

	group_pattern.addOneBind();
	group_pattern.getLastBind() = QueryTree::GroupPattern::Bind(str, var);

	return antlrcpp::Any();
}

/**
	triplesSameSubjectpath : varOrTerm propertyListpathNotEmpty | triplesNodepath 
	propertyListpath ;
	propertyListpath : propertyListpathNotEmpty? ;
	propertyListpathNotEmpty : verbpathOrSimple objectListpath ( ';' ( verbpathOrSimple 
	objectList )? )* ;
	Visit node triplesSameSubjectpath: Add triples to group_pattern.
	Assumptions:
	1) No triplesNodepath in triplesSameSubjectpath;
	2) The child of verbpathOrSimple is verbSimple (no property path, only simple predicate);
	3) The descendant of objectListpath/objectList is varOrTerm (only simple object).

	@param ctx pointer to triplesSameSubjectpath's context.
	@param group_pattern a group graph pattern.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitTriplesSameSubjectpath(SPARQLParser::TriplesSameSubjectpathContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	string subject, predicate, object;

	subject = ctx->varOrTerm()->getText();
	replacePrefix(subject);

	// Assume triplesSameSubjectpath : varOrTerm propertyListpathNotEmpty ;
	auto propertyListpathNotEmpty = ctx->propertyListpathNotEmpty();
	int numChild = propertyListpathNotEmpty->verbpathOrSimple().size();
	for (int i = 0; i < numChild; i++)
	{
		auto verbpathOrSimple = propertyListpathNotEmpty->verbpathOrSimple(i);
		predicate = verbpathOrSimple->getText();
		if (predicate == "a")
			predicate = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>";
		replacePrefix(predicate);

		if (i == 0)
		{
			for (auto objectpath : propertyListpathNotEmpty->objectListpath()->objectpath())
			{
				if (objectpath->graphNodepath()->varOrTerm() \
					&& objectpath->graphNodepath()->varOrTerm()->graphTerm() \
					&& objectpath->graphNodepath()->varOrTerm()->graphTerm()->numericLiteral())
					object = getNumeric(objectpath->graphNodepath()->varOrTerm()->graphTerm()->numericLiteral());
				else if (objectpath->graphNodepath()->varOrTerm() \
					&& objectpath->graphNodepath()->varOrTerm()->graphTerm() \
					&& objectpath->graphNodepath()->varOrTerm()->graphTerm()->booleanLiteral())
					object = "\"" + (objectpath->getText()) + "\"" + "^^<http://www.w3.org/2001/XMLSchema#boolean>";
				else
					object = getTextWithRange(objectpath);
				replacePrefix(object);
				addTriple(subject, predicate, object, group_pattern);
			}
		}
		else
		{
			auto objectList = propertyListpathNotEmpty->objectList(i - 1);
			for (auto object_ptr : objectList->object())
			{
				if (object_ptr->graphNode()->varOrTerm() \
					&& object_ptr->graphNode()->varOrTerm()->graphTerm() \
					&& object_ptr->graphNode()->varOrTerm()->graphTerm()->numericLiteral())
					object = getNumeric(object_ptr->graphNode()->varOrTerm()->graphTerm()->numericLiteral());
				else if (object_ptr->graphNode()->varOrTerm() \
					&& object_ptr->graphNode()->varOrTerm()->graphTerm() \
					&& object_ptr->graphNode()->varOrTerm()->graphTerm()->booleanLiteral())
					object = "\"" + (object_ptr->getText()) + "\"" + "^^<http://www.w3.org/2001/XMLSchema#boolean>";
				else
					object = getTextWithRange(object_ptr);
				replacePrefix(object);
				addTriple(subject, predicate, object, group_pattern);
			}
		}
	}

	return antlrcpp::Any();
}

/**
	Helper function that adds a triple to group_pattern given its subject, predicate, 
	and object; and reorders triples so that FILTER is moved to the back of its scope.
	Called from visitTriplesSameSubjectpath and visitTriplesSameSubject.

	@param subject subject string.
	@param predicate predicate string.
	@param object object string.
	@param group_pattern a group graph pattern.
*/
void QueryParser::addTriple(string subject, string predicate, string object, \
	QueryTree::GroupPattern &group_pattern)
{
	// Add one pattern
	group_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(QueryTree::GroupPattern::Pattern::Element(subject), \
		QueryTree::GroupPattern::Pattern::Element(predicate), \
		QueryTree::GroupPattern::Pattern::Element(object)));

	// Scope of filter
	for (int j = (int)group_pattern.sub_group_pattern.size() - 1; j > 0; j--)
	{
		if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type &&
			group_pattern.sub_group_pattern[j - 1].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
		{
			QueryTree::GroupPattern::SubGroupPattern tmp(group_pattern.sub_group_pattern[j - 1]);
			group_pattern.sub_group_pattern[j - 1] = group_pattern.sub_group_pattern[j];
			group_pattern.sub_group_pattern[j] = tmp;
		}
		else
			break;
	}
}

/**
	groupClause : K_GROUP K_BY groupCondition+ ;
	groupCondition : builtInCall | functionCall | '(' expression ( K_AS var )? ')' | var ;
	Visit node groupClause: Collect GROUP BY variables.
	Assumptions:
	1) The child of groupCondition is var.

	@param ctx pointer to groupClause's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitGroupClause(SPARQLParser::GroupClauseContext *ctx)
{
	for (auto groupCondition : ctx->groupCondition())
	{
		if (groupCondition->children.size() > 1 || !groupCondition->var())
			throw runtime_error("[ERROR]	The supported GROUP BY key is var only.");
		string var = groupCondition->getText();
		query_tree_ptr->addGroupByVar(var);
	}

	return antlrcpp::Any();
}

/**
	orderClause : K_ORDER K_BY orderCondition+ ;
	orderCondition : ( ( K_ASC | K_DESC ) brackettedexpression )| ( constraint | var ) ;
	Visit node orderClause: Collect ORDER BY variables, and whether the order should be 
	ASC or DESC.
	Assumptions:
	1) ORDER BY key can only be var or brackettedexpression.

	@param ctx pointer to orderClause's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitOrderClause(SPARQLParser::OrderClauseContext *ctx)
{
	for (auto orderCondition : ctx->orderCondition())
	{
		bool descending = false;
		string var;
		antlr4::tree::ParseTree *expOrVarCtx;

		// ( 'ASC' | 'DESC' ) brackettedexpression
		if (orderCondition->children[0]->children.size() == 0)
		{
			string tmp = orderCondition->children[0]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp == "DESC")
				descending = true;
		}
		query_tree_ptr->addOrderVar(descending);

		if (orderCondition->children[0]->children.size() == 0)	// ( 'ASC' | 'DESC' ) brackettedexpression
			expOrVarCtx = orderCondition->brackettedexpression()->expression();
		else if (orderCondition->constraint())	// constraint | var
			expOrVarCtx = orderCondition->constraint()->brackettedexpression()->expression();
		else
			expOrVarCtx = orderCondition->var();

		buildCompTree(expOrVarCtx, -1, query_tree_ptr->getLastOrderVar().comp_tree_root);
	}

	return antlrcpp::Any();
}

/**
	Helper function that replaces prefixes in triples with their corresponding IRIs.

	@param str a subject/predicate/object that may contain a prefix.
*/
void QueryParser::replacePrefix(string &str)
{
	if (str[0] != '<' && str[0] != '\"' && str[0] != '?')
	{
		int sep = str.find(":");
		if (sep == -1)	return;

		string prefix = str.substr(0, sep + 1);

		//blank node
		if (prefix == "_:")	return;

		printf("prefix = %s\n", prefix.c_str());
		if (this->prefix_map.count(prefix) != 0)
		{
			str = this->prefix_map[prefix].substr(0, this->prefix_map[prefix].length() - 1) + str.substr(sep + 1 ,str.length() - sep - 1) + ">";
			printf("str = %s\n",str.c_str());
		}
		else
		{
			printf("prefix not found...\n");
			throw runtime_error("[ERROR]	Prefix is not found, please define it before use.");
		}
	}
}

//------------------------------------------------------------------------------------------

/**
	insertData : KK_INSERTDATA quadData ;
	quadData : '{' quads '}' ;
	quads : triplesTemplate? ( quadsNotTriples '.'? triplesTemplate? )* ;
	triplesTemplate : triplesSameSubject ( '.' triplesTemplate? )? ;
	Visit node insertData: set update type as Insert_Data.
	Assumptions:
	1) No quadsNotTriples in quads.

	@param ctx pointer to insertData's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitInsertData(SPARQLParser::InsertDataContext *ctx)
{
	query_tree_ptr->setUpdateType(QueryTree::Insert_Data);

	visit(ctx->quadData());

	return antlrcpp::Any();
}

/**
	deleteData : KK_DELETEDATA quadData ;
	Visit node deleteData: set update type as Delete_Data.
	Assumptions:
	1) No quadsNotTriples in quads.

	@param ctx pointer to deleteData's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitDeleteData(SPARQLParser::DeleteDataContext *ctx)
{
	query_tree_ptr->setUpdateType(QueryTree::Delete_Data);

	visit(ctx->quadData());

	return antlrcpp::Any();
}

/**
	deleteWhere : KK_DELETEWHERE quadPattern ;
	quadPattern : '{' quads '}' ;
	Visit node deleteWhere: set update type as Delete_Where.
	Assumptions:
	1) No quadsNotTriples in quads.

	@param ctx pointer to deleteWhere's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitDeleteWhere(SPARQLParser::DeleteWhereContext *ctx)
{
	query_tree_ptr->setUpdateType(QueryTree::Delete_Where);

	visit(ctx->quadPattern());

	return antlrcpp::Any();
}

/**
	modify : ( K_WITH iri )? ( deleteClause insertClause? | insertClause ) 
	usingClause* K_WHERE groupGraphPattern ;
	Visit node deleteWhere: set update type according to the presence of deleteClause or 
	insertClause, and recursively visit its child groupGraphPattern.
	Assumptions:
	1) No ( K_WITH iri ) in modify.
	2) No usingClause in modify.

	@param ctx pointer to modify's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitModify(SPARQLParser::ModifyContext *ctx)
{
	// ( 'WITH' iri )? not supported

	if (ctx->deleteClause() && ctx->insertClause())
	{
		query_tree_ptr->setUpdateType(QueryTree::Modify_Clause);
		visit(ctx->deleteClause());
		visit(ctx->insertClause());
	}
	else if (ctx->deleteClause())
	{
		query_tree_ptr->setUpdateType(QueryTree::Delete_Clause);
		visit(ctx->deleteClause());
	}
	else if (ctx->insertClause())
	{
		query_tree_ptr->setUpdateType(QueryTree::Insert_Clause);
		visit(ctx->insertClause());
	}

	// usingClause not supported

	visitGroupGraphPattern(ctx->groupGraphPattern(), query_tree_ptr->getGroupPattern());

	return antlrcpp::Any();
}

/**
	triplesSameSubject : varOrTerm propertyListNotEmpty | triplesNode propertyList ;
	propertyList : propertyListNotEmpty? ;
	propertyListNotEmpty : verb objectList ( ';' ( verb objectList )? )* ;
	Visit node triplesSameSubject: Add triples to insert patterns or delete patterns 
	according to the query's update type.
	Assumptions:
	1) No triplesNode in triplesSameSubject;
	2) The descendant of objectList is varOrTerm (only simple object).

	@param ctx pointer to triplesSameSubject's context.
	@return a dummy antlrcpp::Any object.
*/
antlrcpp::Any QueryParser::visitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *ctx)
{
	QueryTree::GroupPattern &group_pattern_insert = query_tree_ptr->getInsertPatterns();
	QueryTree::GroupPattern &group_pattern_delete = query_tree_ptr->getDeletePatterns();

	string subject, predicate, object;

	subject = ctx->varOrTerm()->getText();
	replacePrefix(subject);

	// Assume triplesSameSubject : varOrTerm propertyListNotEmpty ;
	auto propertyListNotEmpty = ctx->propertyListNotEmpty();
	int numChild = propertyListNotEmpty->verb().size();
	for (int i = 0; i < numChild; i++)
	{
		predicate = propertyListNotEmpty->verb(i)->getText();
		auto objectList = propertyListNotEmpty->objectList(i);
		for (auto object_ptr : objectList->object())
		{
			if (object_ptr->graphNode()->varOrTerm() \
				&& object_ptr->graphNode()->varOrTerm()->graphTerm() \
				&& object_ptr->graphNode()->varOrTerm()->graphTerm()->numericLiteral())
				object = getNumeric(object_ptr->graphNode()->varOrTerm()->graphTerm()->numericLiteral());
			else if (object_ptr->graphNode()->varOrTerm() \
				&& object_ptr->graphNode()->varOrTerm()->graphTerm() \
				&& object_ptr->graphNode()->varOrTerm()->graphTerm()->booleanLiteral())
				object = "\"" + (object_ptr->getText()) + "\"" + "^^<http://www.w3.org/2001/XMLSchema#boolean>";
			else
				object = getTextWithRange(object_ptr);
			replacePrefix(object);

			if (query_tree_ptr->getUpdateType() == QueryTree::Delete_Data
				|| query_tree_ptr->getUpdateType() == QueryTree::Delete_Where
				|| query_tree_ptr->getUpdateType() == QueryTree::Delete_Clause)
				addTriple(subject, predicate, object, group_pattern_delete);
			else
				addTriple(subject, predicate, object, group_pattern_insert);
		}
	}

	if (query_tree_ptr->getUpdateType() == QueryTree::Delete_Where)
		query_tree_ptr->getGroupPattern() = query_tree_ptr->getDeletePatterns();

	return antlrcpp::Any();
}

/**
	Helper function that gets a numeric literal from triples and add suffix according 
	to its datatype before storing.

	@param ctx pointer to numericLiteral's context.
	@return string of the numeric literal with suffix added.
*/
string QueryParser::getNumeric(SPARQLParser::NumericLiteralContext *ctx)
{
	int numType = -1;	// 0 for integer, 1 for decimal, 2 for double
	string ret, baseText = ctx->getText();
	
	if (ctx->numericLiteralUnsigned())
	{
		if (ctx->numericLiteralUnsigned()->num_integer())
			numType = 0;
		else if (ctx->numericLiteralUnsigned()->num_decimal())
			numType = 1;
		else if (ctx->numericLiteralUnsigned()->num_double())
			numType = 2;
	}
	else if (ctx->numericLiteralPositive())
	{
		if (ctx->numericLiteralPositive()->integer_positive())
			numType = 0;
		else if (ctx->numericLiteralPositive()->decimal_positive())
			numType = 1;
		else if (ctx->numericLiteralPositive()->double_positive())
			numType = 2;
	}
	else if (ctx->numericLiteralNegative())
	{
		if (ctx->numericLiteralNegative()->integer_negative())
			numType = 0;
		else if (ctx->numericLiteralNegative()->decimal_negative())
			numType = 1;
		else if (ctx->numericLiteralNegative()->double_negative())
			numType = 2;
	}
	switch (numType)
	{
		case 0:
		{
			long long ll;
			bool succ = 1;
			try
			{
				ll = stoll(baseText);
			}
			catch (invalid_argument &e)
			{
				succ = 0;
				throw "[ERROR] xsd:integer value invalid.";
			}
			catch (out_of_range &e)
			{
				succ = 0;
				throw "[ERROR] xsd:integer out of range.";
			}
			if (succ)
				ret = "\"" + baseText + "\"" + "^^<http://www.w3.org/2001/XMLSchema#integer>";
			break;
		}

		case 1:
		ret = "\"" + baseText + "\"" + "^^<http://www.w3.org/2001/XMLSchema#decimal>";
		break;

		case 2:
		ret = "\"" + baseText + "\"" + "^^<http://www.w3.org/2001/XMLSchema#double>";
		break;
	}

	return ret;
}

/**
	Helper function that checks ranges for integer types, and get rids of NaN for float types.
	Wrapper of getText.

	@param ctx pointer to the requested parse tree node's context.
	@return string of the resulting text.
*/

string QueryParser::getTextWithRange(antlr4::tree::ParseTree *ctx)
{
	string ret, baseText = ctx->getText();
	long long ll;
	if (baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#integer>") != string::npos)
	{
		string val = baseText.substr(1, baseText.find("^^<http://www.w3.org/2001/XMLSchema#integer>") - 2);
		try
		{
			ll = stoll(val);
		}
		catch (invalid_argument &e)
		{
			throw "[ERROR] xsd:integer value invalid.";
		}
		catch (out_of_range &e)
		{
			throw "[ERROR] xsd:integer out of range.";
		}
	}
	else if (baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#long>") != string::npos)
	{
		string val = baseText.substr(1, baseText.find("^^<http://www.w3.org/2001/XMLSchema#long>") - 2);
		try
		{
			ll = stoll(val);
		}
		catch (invalid_argument &e)
		{
			throw "[ERROR] xsd:long value invalid.";
		}
		catch (out_of_range &e)
		{
			throw "[ERROR] xsd:long out of range.";
		}
	}
	else if (baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#int>") != string::npos)
	{
		string val = baseText.substr(1, baseText.find("^^<http://www.w3.org/2001/XMLSchema#int>") - 2);
		bool succ = 1;
		try
		{
			ll = stoll(val);
		}
		catch (invalid_argument &e)
		{
			succ = 0;
			throw "[ERROR] xsd:int value invalid.";
		}
		catch (out_of_range &e)
		{
			succ = 0;
			throw "[ERROR] xsd:int out of range.";
		}
		if (succ && (ll < (long long)INT_MIN || ll > (long long)INT_MAX))
			throw "[ERROR] xsd:int out of range.";
	}
	else if (baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#short>") != string::npos)
	{
		string val = baseText.substr(1, baseText.find("^^<http://www.w3.org/2001/XMLSchema#short>") - 2);
		bool succ = 1;
		try
		{
			ll = stoll(val);
		}
		catch (invalid_argument &e)
		{
			succ = 0;
			throw "[ERROR] xsd:short value invalid.";
		}
		catch (out_of_range &e)
		{
			succ = 0;
			throw "[ERROR] xsd:short out of range.";
		}
		if (succ && (ll < (long long)SHRT_MIN || ll > (long long)SHRT_MAX))
			throw "[ERROR] xsd:short out of range.";
	}
	else if (baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#byte>") != string::npos)
	{
		string val = baseText.substr(1, baseText.find("^^<http://www.w3.org/2001/XMLSchema#byte>") - 2);
		bool succ = 1;
		try
		{
			ll = stoll(val);
		}
		catch (invalid_argument &e)
		{
			succ = 0;
			throw "[ERROR] xsd:byte value invalid.";
		}
		catch (out_of_range &e)
		{
			succ = 0;
			throw "[ERROR] xsd:byte out of range.";
		}
		if (succ && (ll < (long long)SCHAR_MIN || ll > (long long)SCHAR_MAX))	// signed char
			throw "[ERROR] xsd:byte out of range.";
	}
	else if (baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#float>") != string::npos
		|| baseText[0] == '"' && baseText.find("^^<http://www.w3.org/2001/XMLSchema#double>") != string::npos)
	{
		if (baseText.substr(1, 3) == "NaN")
			throw "[ERROR] NaN for xsd:float or xsd:double.";
	}

	ret = baseText;
	return ret;
}
