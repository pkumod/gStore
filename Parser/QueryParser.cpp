/*
 * QueryParser.cpp
 *
 *  Created on: 2015-4-11
 *      Author: cjq
 */

#include "QueryParser.h"

using namespace std;

QueryParser::QueryParser()
{
	_prefix_map.clear();
}

void QueryParser::sparqlParser(const string& query, QueryTree& querytree)
{
	//uncompress before use
	dfa34_Table_uncompress();

	pANTLR3_INPUT_STREAM input;
	pSparqlLexer lex;
	pANTLR3_COMMON_TOKEN_STREAM tokens;
	pSparqlParser parser;
	input = antlr3StringStreamNew((ANTLR3_UINT8 *)(query.c_str()), ANTLR3_ENC_UTF8, query.length(), (ANTLR3_UINT8 *)"QueryString");
	//input = antlr3FileStreamNew((pANTLR3_UINT8)filePath,ANTLR3_ENC_8BIT);
	lex = SparqlLexerNew(input);

	tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,TOKENSOURCE(lex));
	parser = SparqlParserNew(tokens);

	SparqlParser_workload_return r = parser->workload(parser);
	pANTLR3_BASE_TREE root = r.tree;

	if (printNode(root) > 0)	throw "Some errors are found in the SPARQL query request.";

	parseTree(root, querytree);

	printQuery(querytree);

	parser->free(parser);
	tokens->free(tokens);
	lex->free(lex);
	input->close(input);
}

int QueryParser::printNode(pANTLR3_BASE_TREE node, int dep)
{
	const char* s = (const char*) node->getText(node)->chars;
	ANTLR3_UINT32 treeType = node->getType(node);

	int hasErrorNode = 0;
	if (treeType == 0)	hasErrorNode = 1;

	for (int i=0; i < dep; i++)		printf("    ");
	printf("%d: %s\n",treeType,s);

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);
		hasErrorNode += printNode(childNode, dep + 1);
	}
	return hasErrorNode;
}


void QueryParser::parseTree(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
	printf("parseTree\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
			pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, i);

			//prologue	144
			if (childNode->getType(childNode) == 144)
			{
				parsePrologue(childNode);
			}
			else
			//select 155
			if (childNode->getType(childNode) == 155)
			{
				querytree.setQueryForm(QueryTree::Select_Query);
				parseTree(childNode, querytree);
			}
			else
			//ask 13
			if (childNode->getType(childNode) == 13)
			{
				querytree.setQueryForm(QueryTree::Ask_Query);
				parseTree(childNode, querytree);
			}
			else
			//select clause 156
			if (childNode->getType(childNode) == 156)
			{
				parseSelectClause(childNode, querytree);
			}
			else
			//group graph pattern 77
			if (childNode->getType(childNode) == 77)
			{
				parseGroupPattern(childNode, querytree.getPatternGroup());
			}
			else
			//order by 127
			if (childNode->getType(childNode) == 127)
			{
				parseOrderBy(childNode, querytree);
			}
			else
			//offset 120	limit 102
			if (childNode->getType(childNode) == 120 || childNode->getType(childNode) == 102)
			{
				pANTLR3_BASE_TREE gchildNode=(pANTLR3_BASE_TREE) childNode->getChild(childNode, 0);

				//integer 83
				if (gchildNode->getType(gchildNode) == 83)
				{
					string str;
					parseString(gchildNode, str, 0);

					stringstream str2int;

					int num;

					str2int << str;
					str2int >> num;

					if (childNode->getType(childNode) == 120 && num >= 0)
						querytree.setOffset(num);
					if (childNode->getType(childNode) == 102 && num >= 0)
						querytree.setLimit(num);
				}
			}
			else	parseTree(childNode, querytree);
	}
}
void QueryParser::parsePrologue(pANTLR3_BASE_TREE node)
{
	printf("parsePrologue\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
			pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, i);

			//prefix 143
			if (childNode->getType(childNode) == 143)
				parsePrefix(childNode);
	}
}
void QueryParser::parsePrefix(pANTLR3_BASE_TREE node)
{
	printf("parsePrefix\n");

	string key;
	string value;

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);
		//prefix namespace 136
		if (childNode->getType(childNode) == 136)
			parseString(childNode, key, 0);

		//prefix IRI 89
		if (childNode->getType(childNode) == 89)
			parseString(childNode, value, 0);
	}
	_prefix_map.insert(make_pair(key, value));
}

void QueryParser::replacePrefix(string& str)
{
	if (str[0] != '<' && str[0] != '\"' && str[0] != '?')
	{
		int sep=str.find(":");
		if (sep == -1)	return;
		string prefix=str.substr(0, sep + 1);

		//blank node
		if (prefix == "_:")	return;

		cout << "prefix: " << prefix << endl;
		if (_prefix_map.find(prefix) != _prefix_map.end())
		{
			str=_prefix_map[prefix].substr(0, _prefix_map[prefix].length() - 1) + str.substr(sep + 1 ,str.length() - sep - 1) + ">";
			cout << "str: " << str << endl;
		}
		else
		{
			cout << "prefix not found..." << endl;
			throw "Some errors are found in the SPARQL query request.";
		}
	}
}

void QueryParser::parseSelectClause(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
	printf("parseSelectClause\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, i);

		//distinct 52
		if (childNode->getType(childNode) == 52)
			querytree.setProjectionModifier(QueryTree::Modifier_Distinct);

		//var 199
		if (childNode->getType(childNode) == 199)
			parseSelectVar(childNode, querytree);

		//asterisk 14
		if (childNode->getType(childNode) == 14)
			querytree.setProjectionAsterisk();
	}
}

void QueryParser::parseSelectVar(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
	printf("parseSelectVar\n");

	string var = "";
	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		if (childNode->getType(childNode) == 200)
		{
			parseString(childNode, var, 0);
			querytree.addProjectionVar(var);
		}
	}
}

void QueryParser::parseGroupPattern(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup)
{
	printf("parseGroupPattern\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//triples same subject 185
		if (childNode->getType(childNode) == 185)
		{
			parsePattern(childNode, patterngroup);
		}

		//optional	124		minus	108
		if (childNode->getType(childNode) == 124 || childNode->getType(childNode) == 108)
		{
			parseOptionalOrMinus(childNode, patterngroup);
		}

		//union  195
		if (childNode->getType(childNode) == 195)
		{
			patterngroup.addOneGroupUnion();
			parseUnion(childNode, patterngroup);
		}

		//filter 67
		if (childNode->getType(childNode) == 67)
		{
			parseFilter(childNode, patterngroup);
		}

		//group graph pattern 77
		//redundant {}
		if (childNode->getType(childNode) == 77)
		{
			parseGroupPattern(childNode, patterngroup);
		}
	}
}

void QueryParser::parsePattern(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup)
{
	printf("parsePattern\n");

	string subject = "";
	string predicate = "";
	string object = "";
	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//subject 177
		if (childNode->getType(childNode) == 177)
		{
			parseString(childNode, subject, 1);
			replacePrefix(subject);
		}

		//predicate 142
		if (childNode->getType(childNode) == 142)
		{
			parseString(childNode, predicate, 4);
			replacePrefix(predicate);
		}

		//object 119
		if (childNode->getType(childNode) == 119)
		{
			parseString(childNode, object, 1);
			replacePrefix(object);
		}

		if (i != 0 && i % 2 == 0)		//triples same subject
		{
			patterngroup.addOnePattern(QueryTree::Pattern(QueryTree::Element(subject), QueryTree::Element(predicate), QueryTree::Element(object)));
		}
	}
}

void QueryParser::parseOptionalOrMinus(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup)
{
	//optional	124		minus	108
	if (node->getType(node) == 124)
		printf("parseOptional\n");
	else if (node->getType(node) == 108)
		printf("parseMinus\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//group graph pattern 77
		if (childNode->getType(childNode) == 77)
		{
			if (node->getType(node) == 124)
				patterngroup.addOneOptionalOrMinus('o');
			else if (node->getType(node) == 108)
				patterngroup.addOneOptionalOrMinus('m');
			parseGroupPattern(childNode, patterngroup.getLastOptionalOrMinus());
		}
	}
}

void QueryParser::parseUnion(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup)
{
	printf("parseUnion\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//group graph pattern 77
		if (childNode->getType(childNode) == 77)
		{
			patterngroup.addOneUnion();
			parseGroupPattern(childNode, patterngroup.getLastUnion());
		}

		//union  195
		if (childNode->getType(childNode) == 195)
		{
			parseUnion(childNode, patterngroup);
		}
	}
}

void QueryParser::parseFilter(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup)
{
	printf("parseFilter\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//unary 190
		if (childNode->getType(childNode) == 190)
			childNode = (pANTLR3_BASE_TREE) childNode->getChild(childNode, 0);

		patterngroup.addOneFilterTree();
		parseFilterTree(childNode, patterngroup, patterngroup.getLastFilterTree());
	}
}

void QueryParser::parseFilterTree(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup, QueryTree::FilterTree& filter)
{
	printf("parseFilterTree\n");

	switch (node->getType(node))
	{
		//! 192
		case 192:	filter.type = QueryTree::FilterTree::Not_type;		break;
		//not 115
		case 115:	filter.type = QueryTree::FilterTree::Not_type;		break;
		//or 125
		case 125:	filter.type = QueryTree::FilterTree::Or_type;		break;
		//and 8
		case 8:		filter.type = QueryTree::FilterTree::And_type;		break;
		//equal 62
		case 62:		filter.type = QueryTree::FilterTree::Equal_type;		break;
		//not equal 116
		case 116:	filter.type = QueryTree::FilterTree::NotEqual_type;		break;
		//less 100
		case 100:	filter.type = QueryTree::FilterTree::Less_type;		break;
		//less equal 101
		case 101:	filter.type = QueryTree::FilterTree::LessOrEqual_type;		break;
		//greater 72
		case 72:		filter.type = QueryTree::FilterTree::Greater_type;		break;
		//greater equal 73
		case 73:		filter.type = QueryTree::FilterTree::GreaterOrEqual_type;		break;

		//regex 150
		case 150:	filter.type = QueryTree::FilterTree::Builtin_regex_type;		break;
		//lang 96
		case 96:		filter.type = QueryTree::FilterTree::Builtin_lang_type;		break;
		//langmatches 97
		case 97:		filter.type = QueryTree::FilterTree::Builtin_langmatches_type;		break;
		//bound 23
		case 23:		filter.type = QueryTree::FilterTree::Builtin_bound_type;		break;
		//in 81
		case 81:		filter.type = QueryTree::FilterTree::Builtin_in_type;		break;
		//exists 63
		case 63:		filter.type = QueryTree::FilterTree::Builtin_exists_type;		break;
		//not exists 117
		case 117:	filter.type = QueryTree::FilterTree::Not_type;		break;

		default:
			return;
	}

	//in the "NOT IN" case,  in, var and expression list is on the same layer.
	//not 115
	if (node->getType(node) == 115)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, 0);

		//in 81
		if (childNode->getType(childNode) == 81)
		{
			filter.child.push_back(QueryTree::FilterTree::FilterTreeChild());
			filter.child[0].type = 't';
			filter.child[0].ptr = new QueryTree::FilterTree();
			filter.child[0].ptr->type = QueryTree::FilterTree::Builtin_in_type;
			parseVarInExpressionList(node, *filter.child[0].ptr, 1);

			return;
		}
	}

	//in 81
	if (node->getType(node) == 81)
	{
		parseVarInExpressionList(node, filter, 0);

		return;
	}

	//not exists 117
	if (node->getType(node) == 117)
	{
		filter.child.push_back(QueryTree::FilterTree::FilterTreeChild());
		filter.child[0].type = 't';
		filter.child[0].ptr = new QueryTree::FilterTree();
		filter.child[0].ptr->type = QueryTree::FilterTree::Builtin_exists_type;

		parseExistsGroupPattern(node, patterngroup, *filter.child[0].ptr);

		return;
	}

	//exists 63
	if (node->getType(node) == 63)
	{
		parseExistsGroupPattern(node, patterngroup, filter);

		return;
	}

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//unary 190
		if (childNode->getType(childNode) == 190)
		{
			pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE) childNode->getChild(childNode, 0);
			//unsigned int type = gchildNode->getType(gchildNode);
			//regex 150	lang 96	langmatches 97	bound 23	exists 63
			//if (type == 150 || type == 96 || type == 97 || type == 23 || type == 63)
			if (gchildNode->getChildCount(gchildNode) != 0)
				childNode = gchildNode;
		}

		filter.child.push_back(QueryTree::FilterTree::FilterTreeChild());

		//unary 190
		if (childNode->getType(childNode) == 190)
		{
			filter.child[i].type = 's';
			parseString(childNode, filter.child[i].arg, 1);
			replacePrefix(filter.child[i].arg);
		}
		else if (childNode->getChildCount(childNode) == 0)
		{
			filter.child[i].type = 's';
			parseString(childNode, filter.child[i].arg, 0);
			replacePrefix(filter.child[i].arg);
		}
		else
		{
			filter.child[i].type = 't';
			filter.child[i].ptr = new QueryTree::FilterTree();
			parseFilterTree(childNode, patterngroup, *filter.child[i].ptr);
		}
	}
}

void QueryParser::parseVarInExpressionList(pANTLR3_BASE_TREE node, QueryTree::FilterTree& filter, unsigned int begin)
{
	printf("parseVarInExpressionList\n");

	for (unsigned int i = begin; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//unary 190
		if (childNode->getType(childNode) == 190)
		{
			filter.child.push_back(QueryTree::FilterTree::FilterTreeChild());

			filter.child[i - begin].type = 's';
			parseString(childNode, filter.child[i - begin].arg, 1);
			replacePrefix(filter.child[i - begin].arg);
		}

		//expression list 65
		if (childNode->getType(childNode) == 65)
		{
			for (unsigned int j = 0; j < childNode->getChildCount(childNode); j++)
			{
				pANTLR3_BASE_TREE gchildNode=(pANTLR3_BASE_TREE) childNode->getChild(childNode, j);

				filter.child.push_back(QueryTree::FilterTree::FilterTreeChild());

				filter.child[i + j - begin].type = 's';
				parseString(gchildNode, filter.child[i + j - begin].arg, 1);
				replacePrefix(filter.child[i + j - begin].arg);
			}
		}
	}
}

void QueryParser::parseExistsGroupPattern(pANTLR3_BASE_TREE node, QueryTree::PatternGroup& patterngroup, QueryTree::FilterTree& filter)
{
	printf("parseExistsGroupPattern\n");

	pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, 0);

	//group graph pattern 77
	if (childNode->getType(childNode) == 77)
	{
		patterngroup.addOneExistsGroupPattern();
		filter.exists_patterngroup_id = (int)patterngroup.filter_exists_patterngroups[(int)patterngroup.filter_exists_patterngroups.size() - 1].size() - 1;
		parseGroupPattern(childNode, patterngroup.getLastExistsGroupPattern());

		return;
	}
}

void QueryParser::parseOrderBy(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
	printf("parseOrderBy\n");

	for (unsigned int i = 0; i < node->getChildCount(node); i++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, i);

		//order by condition
		if (childNode->getType(childNode) == 128)
		{
			string var;
			bool desending = false;
			for (unsigned int k = 0; k < childNode->getChildCount(childNode); k++)
			{
				pANTLR3_BASE_TREE gchildNode=(pANTLR3_BASE_TREE) childNode->getChild(childNode, k);

				//var 200
				if (gchildNode->getType(gchildNode) == 200)
					parseString(gchildNode, var, 0);

				//unary 190
				if (gchildNode->getType(gchildNode) == 190)
					parseString(gchildNode, var, 1);

				//asend 12
				if (gchildNode->getType(gchildNode) == 12)
					desending = false;

				//desend 49
				if (gchildNode->getType(gchildNode) == 49)
					desending = true;
			}

			querytree.addOrder(var, desending);
		}
	}
}

void QueryParser::parseString(pANTLR3_BASE_TREE node, string& str, int dep)
{
	if (dep == 0)
	{
		str = (const char*) node->getText(node)->chars;
		return;
	}

	while (dep > 1 && node != NULL)
	{
		node = (pANTLR3_BASE_TREE) node->getChild(node, 0);
		dep--;
	}

	if (node == NULL || node->getChildCount(node) == 0)
		throw "Some errors are found in the SPARQL query request.";
	else
	{
		for (unsigned int i = 0; i < node->getChildCount(node); i++)
		{
			pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, i);

			unsigned int type = childNode->getType(childNode);

			//var 200	string literal 170(single quotation marks) 171(double quotation marks)
			//IRI 89		PNAME_LN 135
			//if (type == 200 || type == 170 || type == 171 || type == 89 || type == 135)
			if (type != 98)
			{
				str = (const char*) childNode->getText(childNode)->chars;
				if (type == 170)
					str = "\"" + str.substr(1, str.length() - 2) + "\"";
			}

			//custom language 98
			if (type == 98)
			{
				string custom_lang;
				custom_lang = (const char*) childNode->getText(childNode)->chars;
				str += custom_lang;
			}
		}
	}
}


void QueryParser::printQuery(QueryTree& querytree)
{
	printf("===========================================================================\n");

	if (querytree.getQueryForm() == QueryTree::Select_Query)
	{
		printf("select");
		if (querytree.getProjectionModifier() == QueryTree::Modifier_Distinct)
			printf(" distinct");
		printf("\n");

		printf("var is : \t");
		vector <string> &varvec = querytree.getProjection().varset;
		for (int i = 0; i < (int)varvec.size(); i++)
			printf("%s\t", varvec[i].c_str());
		if (querytree.checkProjectionAsterisk())
			printf("*");
		printf("\n");
	}
	else printf("ask\n");

	printPatternGroup(querytree.getPatternGroup(), 0);

	if ((int)querytree.getOrder().size() > 0)
	{
		printf("order by : \t");

		vector<QueryTree::Order>&order = querytree.getOrder();
		for (int i = 0; i < (int)order.size(); i++)
		{
			if (!order[i].descending)	printf("ASC(");
			else printf("DESC(");
			printf("%s)  ", order[i].var.c_str());
		}
		printf("\n");
	}
	if (querytree.getOffset() != 0)
		printf("offset : %d\n", querytree.getOffset());
	if (querytree.getLimit() != -1)
		printf("limit : %d\n", querytree.getLimit());
	printf("===========================================================================\n");
}

void QueryParser::printPatternGroup(QueryTree::PatternGroup &patterngroup, int dep)
{
	for (int j = 0; j < dep; j++)	printf("\t");	printf("{\n");
	for (int j = 0; j < dep; j++)	printf("\t");	printf("pattern:\n");
	for(int i = 0; i < (int)patterngroup.patterns.size(); i++)
	{
		for (int j = 0; j < dep; j++)	printf("\t");
		printf("\t%s\t%s\t%s\n", patterngroup.patterns[i].subject.value.c_str(), patterngroup.patterns[i].predicate.value.c_str(), patterngroup.patterns[i].object.value.c_str());
	}

	for (int i = 0; i < (int)patterngroup.unions.size(); i++)
	{
		for (int j = 0; j < dep; j++)	printf("\t");	printf("union %d:\n", i);
		for (int j = 0; j < (int)patterngroup.unions[i].size(); j++)
			printPatternGroup(patterngroup.unions[i][j], dep + 1);
	}

	if ((int)patterngroup.optionals.size() > 0)
	{
		for (int j = 0; j < dep; j++)	printf("\t");		printf("optional:\n");
		for (int i = 0; i < (int)patterngroup.optionals.size(); i++)
		{
			printf("\tlast pattern id = %d\tlast union id = %d\n", patterngroup.optionals[i].lastpattern, patterngroup.optionals[i].lastunions);
			printPatternGroup(patterngroup.optionals[i].patterngroup, dep + 1);
		}
	}

	if ((int)patterngroup.filters.size() > 0)
	{
		for (int j = 0; j < dep; j++)	printf("\t");		printf("filter:\n");
		for (int i = 0; i < (int)patterngroup.filters.size(); i++)
		{
			for (int j = 0; j <= dep; j++)	printf("\t");
			printFilter(patterngroup.filter_exists_patterngroups[i], patterngroup.filters[i], dep + 1);
			printf("\n");
		}
	}

	for (int j = 0; j < dep; j++)	printf("\t");		printf("}\n");
}

void QueryParser::printFilter(std::vector<QueryTree::PatternGroup> &exist_patterngroups, QueryTree::FilterTree &filter, int dep)
{
	if (filter.type == QueryTree::FilterTree::Not_type)	printf("!");
	if (filter.type == QueryTree::FilterTree::Builtin_regex_type)	printf("regex");
	if (filter.type == QueryTree::FilterTree::Builtin_lang_type)		printf("lang");
	if (filter.type == QueryTree::FilterTree::Builtin_langmatches_type)		printf("langmatches");
	if (filter.type == QueryTree::FilterTree::Builtin_bound_type)		printf("bound");

	if (filter.type == QueryTree::FilterTree::Builtin_in_type)
	{
		if (filter.child[0].type == 's')			printf("%s", filter.child[0].arg.c_str());
		printf(" in (");
		for (int i = 1; i < (int)filter.child.size(); i++)
		{
			if (i != 1)	printf(" , ");
			if (filter.child[i].type == 's')		printf("%s", filter.child[i].arg.c_str());
		}
		printf(")");

		return;
	}

	if (filter.type == QueryTree::FilterTree::Builtin_exists_type)
	{
		printf("exists");
		printPatternGroup(exist_patterngroups[filter.exists_patterngroup_id], dep);

		return;
	}

	printf("(");

	if ((int)filter.child.size() >= 1)
	{
		if (filter.child[0].type == 's')			printf("%s", filter.child[0].arg.c_str());
		if (filter.child[0].type == 't')			printFilter(exist_patterngroups, *filter.child[0].ptr, dep);
	}

	if (filter.type == QueryTree::FilterTree::Or_type)	printf(" || ");
	if (filter.type == QueryTree::FilterTree::And_type)	printf(" && ");
	if (filter.type == QueryTree::FilterTree::Equal_type)	printf(" = ");
	if (filter.type == QueryTree::FilterTree::NotEqual_type)	printf(" != ");
	if (filter.type == QueryTree::FilterTree::Less_type)	printf(" < ");
	if (filter.type == QueryTree::FilterTree::LessOrEqual_type)	printf(" <= ");
	if (filter.type == QueryTree::FilterTree::Greater_type)	printf(" > ");
	if (filter.type == QueryTree::FilterTree::GreaterOrEqual_type)	printf(" >= ");

	if (filter.type == QueryTree::FilterTree::Builtin_regex_type || filter.type == QueryTree::FilterTree::Builtin_langmatches_type)	printf(", ");

	if ((int)filter.child.size() >= 2)
	{
		if (filter.child[1].type == 's')			printf("%s", filter.child[1].arg.c_str());
		if (filter.child[1].type == 't')			printFilter(exist_patterngroups, *filter.child[1].ptr, dep);
	}

	if ((int)filter.child.size() >= 3)
	{
		if (filter.type == QueryTree::FilterTree::Builtin_regex_type && filter.child[2].type == 's')
			printf(", %s", filter.child[2].arg.c_str());
	}

	printf(")");
}
