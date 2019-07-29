#include "QueryParser.h"

using namespace std;

void SPARQLErrorListener::syntaxError(antlr4::Recognizer *recognizer, antlr4::Token * offendingSymbol, \
	size_t line, size_t charPositionInLine, const std::string &msg, std::exception_ptr e)
{
	throw runtime_error("line " + to_string(line) + ":" + to_string(charPositionInLine) + " " + msg);
}

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
	visitEntry(tree);
}

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

void QueryParser::printQueryTree()
{
	query_tree_ptr->print();
}

antlrcpp::Any QueryParser::visitQueryUnit(SPARQLParser::QueryUnitContext *ctx)
{
	// printNode(ctx, "queryUnit");

	visit(ctx->query());

	return antlrcpp::Any();
}

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

antlrcpp::Any QueryParser::visitSelectquery(SPARQLParser::SelectqueryContext *ctx)
{
	visit(ctx->selectClause());

	// datasetClause not supported

	// Call explictly to pass group_pattern as a parameter
	visitWhereClause(ctx->whereClause(), query_tree_ptr->getGroupPattern());

	visit(ctx->solutionModifier());

	return antlrcpp::Any();
}

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

antlrcpp::Any QueryParser::visitWhereClause(SPARQLParser::WhereClauseContext *ctx, QueryTree::GroupPattern &group_pattern)
{
	// Call explictly to pass group_pattern as a parameter
	visitGroupGraphPattern(ctx->groupGraphPattern(), group_pattern);

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitLimitClause(SPARQLParser::LimitClauseContext *ctx)
{
	query_tree_ptr->setLimit(stoi(ctx->children[1]->getText()));

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitOffsetClause(SPARQLParser::OffsetClauseContext *ctx)
{
	query_tree_ptr->setOffset(stoi(ctx->children[1]->getText()));

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitPrefixDecl(SPARQLParser::PrefixDeclContext *ctx)
{
	string key, value;

	key = ctx->PNAME_NS()->getText();
	value = ctx->IRIREF()->getText();

	prefix_map[key] = value;

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitSelectClause(SPARQLParser::SelectClauseContext *ctx)
{
	// printNode(ctx, "selectClause");

	if (ctx->children[1]->children.size() == 0)
	{
		string tmp = ctx->children[1]->getText();
		transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
		if (ctx->children[1]->getText() == "DISTINCT")
			query_tree_ptr->setProjectionModifier(QueryTree::Modifier_Distinct);
		else if (ctx->children[1]->getText() == "REDUCED")
			throw runtime_error("[ERROR]	REDUCED is currently not supported.");
		else if (ctx->children[1]->getText() == "*")
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

void QueryParser::parseSelectAggregateFunction(SPARQLParser::ExpressionContext *expCtx, \
	SPARQLParser::VarContext *varCtx)
{
	// Sanity checks
	SPARQLParser::BuiltInCallContext *bicCtx = expCtx->conditionalOrexpression()-> \
		conditionalAndexpression(0)->valueLogical(0)->relationalexpression()-> \
		numericexpression(0)->additiveexpression()->multiplicativeexpression(0)-> \
		unaryexpression(0)->primaryexpression()->builtInCall();
	if (!bicCtx)
		throw runtime_error("[ERROR]	Currently only support selecting variables and "
			"the aggregate function COUNT");
	antlr4::tree::ParseTree *curr = expCtx;
	for (int i = 0; i < 11; i++)
	{
		// Make sure only one children along the way
		if (curr->children.size() > 1)
			throw runtime_error("[ERROR]	Currently only support selecting variables and "
				"the aggregate function COUNT");
		curr = curr->children[0];
	}
	SPARQLParser::AggregateContext *aggCtx = bicCtx->aggregate();
	if (!aggCtx)
		throw runtime_error("[ERROR]	Currently only support selecting variables and "
			"the aggregate function COUNT");
	string tmp = aggCtx->children[0]->getText();
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	if (aggCtx->children[0]->getText() != "COUNT")
		throw runtime_error("[ERROR]	The supported aggregate function now is COUNT only");

	query_tree_ptr->addProjectionVar();
	QueryTree::ProjectionVar &proj_var = query_tree_ptr->getLastProjectionVar();
	proj_var.aggregate_type = QueryTree::ProjectionVar::Count_type;
	proj_var.aggregate_var = aggCtx->expression()->getText();	// Error would have been dealt with
	tmp = aggCtx->children[2]->getText();
	transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
	proj_var.distinct = aggCtx->children[2]->children.size() == 0 && tmp == "DISTINCT";
	proj_var.var = varCtx->getText();
}

antlrcpp::Any QueryParser::visitGroupGraphPattern(SPARQLParser::GroupGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	// subSelect not supported

	visitGroupGraphPatternSub(ctx->groupGraphPatternSub(), group_pattern);

	return antlrcpp::Any(); 
}

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

antlrcpp::Any QueryParser::visitTriplesBlock(SPARQLParser::TriplesBlockContext *ctx, QueryTree::GroupPattern &group_pattern)
{
	visitTriplesSameSubjectpath(ctx->triplesSameSubjectpath(), group_pattern);
	if (ctx->triplesBlock())
		visitTriplesBlock(ctx->triplesBlock(), group_pattern);

	return antlrcpp::Any();
}

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

antlrcpp::Any QueryParser::visitGroupOrUnionGraphPattern(SPARQLParser::GroupOrUnionGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	if (ctx->children.size() == 1)
		visitGroupGraphPattern(ctx->groupGraphPattern(0), group_pattern);
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

antlrcpp::Any QueryParser::visitOptionalGraphPattern(SPARQLParser::OptionalGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	group_pattern.addOneOptional(QueryTree::GroupPattern::SubGroupPattern::Optional_type);
	visitGroupGraphPattern(ctx->groupGraphPattern(), group_pattern.getLastOptional());

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitMinusGraphPattern(SPARQLParser::MinusGraphPatternContext *ctx, \
	QueryTree::GroupPattern &group_pattern)
{
	group_pattern.addOneOptional(QueryTree::GroupPattern::SubGroupPattern::Minus_type);
	visitGroupGraphPattern(ctx->groupGraphPattern(), group_pattern.getLastOptional());

	return antlrcpp::Any();
}

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
		else	// iriOrFunction | rDFLiteral | numericLiteral | booleanLiteral | var
		{
			if (currChild)	// This SHOULD hold true, unless someone only writes one var or literal 
				// inside brackets to constitute brackettedexpression, which is meaningless
			{
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
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
			else
				throw runtime_error("[ERROR] Filter currently does not support this built-in call.");

			if (currChild)
				currChild->node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;

			tmp = root->children[0]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp != "BOUND")
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
			else
			{
				filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
				filter.child[0].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
				filter.child[0].str = ((SPARQLParser::BuiltInCallContext *)root)->var()->getText();
				replacePrefix(filter.child[0].str);
			}
		}
		else
			throw runtime_error("[ERROR]	Filter currently does not support this built-in call.");
	}
	else
		throw runtime_error("[ERROR]	Unaccounted for route when parsing filter.");
}

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
				object = objectpath->getText();
				replacePrefix(object);
				addTriple(subject, predicate, object, group_pattern);
			}
		}
		else
		{
			auto objectList = propertyListpathNotEmpty->objectList(i - 1);
			for (auto object_ptr : objectList->object())
			{
				object = object_ptr->getText();
				replacePrefix(object);
				addTriple(subject, predicate, object, group_pattern);
			}
		}
	}

	return antlrcpp::Any();
}

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

antlrcpp::Any QueryParser::visitOrderClause(SPARQLParser::OrderClauseContext *ctx)
{
	for (auto orderCondition : ctx->orderCondition())
	{
		bool descending = false;
		string var;

		// ( 'ASC' | 'DESC' ) brackettedexpression
		if (orderCondition->children[0]->children.size() == 0)
		{
			string tmp = orderCondition->children[0]->getText();
			transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
			if (tmp == "DESC")
				descending = true;

			SPARQLParser::VarContext *varCtx = orderCondition->brackettedexpression()->expression()-> \
				conditionalOrexpression()->conditionalAndexpression(0)->valueLogical(0)-> \
				relationalexpression()->numericexpression(0)->additiveexpression()-> \
				multiplicativeexpression(0)->unaryexpression(0)->primaryexpression()->var();
			if (!varCtx)
				throw runtime_error("[ERROR]	The supported ORDER BY key is var only.");
			antlr4::tree::ParseTree *curr = orderCondition->brackettedexpression()->expression();
			for (int i = 0; i < 10; i++)
			{
				// Make sure only one children along the way
				if (curr->children.size() > 1)
					throw runtime_error("[ERROR]	The supported ORDER BY key is var only.");
				curr = curr->children[0];
			}
			var = varCtx->getText();
		}
		// constraint | var
		else
		{
			if (!orderCondition->var())
				throw runtime_error("[ERROR]	The supported ORDER BY key is var only.");
			var = orderCondition->var()->getText();
		}

		query_tree_ptr->addOrderVar(var, descending);
	}

	return antlrcpp::Any();
}

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

antlrcpp::Any QueryParser::visitInsertData(SPARQLParser::InsertDataContext *ctx)
{
	query_tree_ptr->setUpdateType(QueryTree::Insert_Data);

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitDeleteData(SPARQLParser::DeleteDataContext *ctx)
{
	query_tree_ptr->setUpdateType(QueryTree::Delete_Data);

	return antlrcpp::Any();
}

antlrcpp::Any QueryParser::visitDeleteWhere(SPARQLParser::DeleteWhereContext *ctx)
{
	query_tree_ptr->setUpdateType(QueryTree::Delete_Where);

	return antlrcpp::Any();
}

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

antlrcpp::Any QueryParser::visitTriplesSameSubject(SPARQLParser::TriplesSameSubjectContext *ctx)
{
	QueryTree::GroupPattern &group_pattern = query_tree_ptr->getInsertPatterns();
	if (query_tree_ptr->getUpdateType() == QueryTree::Delete_Data
		|| query_tree_ptr->getUpdateType() == QueryTree::Delete_Where
		|| query_tree_ptr->getUpdateType() == QueryTree::Delete_Clause)
		group_pattern = query_tree_ptr->getDeletePatterns();
	// QueryTree::Insert_Data, QueryTree::Insert_Clause, QueryTree::Modify_Clause

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
			object = object_ptr->getText();
			replacePrefix(object);
			addTriple(subject, predicate, object, group_pattern);
		}
	}

	if (query_tree_ptr->getUpdateType() == QueryTree::Delete_Where)
		query_tree_ptr->getGroupPattern() = query_tree_ptr->getDeletePatterns();

	return antlrcpp::Any();
}

