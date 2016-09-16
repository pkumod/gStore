/*
 * DBparser.cpp
 *
 *  Created on: 2015-4-11
 *      Author: cjq
 */

#include "DBparser.h"


DBparser::DBparser()
{
	_prefix_map.clear();
}

void DBparser::sparqlParser(const std::string& _sparql, SPARQLquery& _sparql_query)
{
	pANTLR3_INPUT_STREAM input;
	pSparqlLexer lex;
	pANTLR3_COMMON_TOKEN_STREAM tokens;
	pSparqlParser parser;
	input = antlr3StringStreamNew((ANTLR3_UINT8 *)(_sparql.c_str()), ANTLR3_ENC_UTF8, _sparql.length(), (ANTLR3_UINT8 *)"QueryString");
	//input = antlr3FileStreamNew((pANTLR3_UINT8)filePath,ANTLR3_ENC_8BIT);
	lex = SparqlLexerNew(input);

	tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT,TOKENSOURCE(lex));
	parser = SparqlParserNew(tokens);


	SparqlParser_workload_return r = parser->workload(parser);
	pANTLR3_BASE_TREE root = r.tree;

	if (printNode(root) > 0)	throw "Some errors are found in the SPARQL query request.";
	parseTree(root,_sparql_query);

	printquery(_sparql_query);

	genQueryVec(_sparql_query.getPatternGroup(), _sparql_query);

	parser->free(parser);
	tokens->free(tokens);
	lex->free(lex);
	input->close(input);
}

int DBparser::printNode(pANTLR3_BASE_TREE node, int depth)
{
	const char* s = (const char*) node->getText(node)->chars;
	ANTLR3_UINT32 treeType = node->getType(node);

	int hasErrorNode = 0;
	if (treeType == 0)	hasErrorNode = 1;

	for (int i=0; i < depth; i++)		printf("    ");
	printf("%d: %s\n",treeType,s);
	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);
		int childNodeType = childNode->getType(childNode);
		hasErrorNode += printNode(childNode, depth+1);
	}
	return hasErrorNode;
}


void DBparser::parseTree(pANTLR3_BASE_TREE node, SPARQLquery& query)
{
	printf("parseTree\n");
	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
			pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, j);

			//prologue	144
			if (childNode->getType(childNode) == 144)
			{
				parsePrologue(childNode);
			}
			else
			//select clause 156
			if (childNode->getType(childNode) == 156)
			{
					parseSelectClause(childNode, query);
			}
			else
			//group graph pattern 77
			if (childNode->getType(childNode) == 77)
			{
					parseGroupPattern(childNode, query.getPatternGroup());
			}
			else	parseTree(childNode, query);
	}
}
void DBparser::parsePrologue(pANTLR3_BASE_TREE node)
{
	printf("parsePrologue\n");
	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
			pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, j);

			//prefix 143
			if (childNode->getType(childNode) == 143)
			{
				parsePrefix(childNode);
			}
	}
}
void DBparser::parsePrefix(pANTLR3_BASE_TREE node)
{
	printf("parsePrefix\n");

	std::string key;
	std::string value;

	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);
		//prefix key string 136
		if (childNode->getType(childNode) == 136)
		{
			parseString(childNode, key);
		}

		//prefix value URL 89
		if (childNode->getType(childNode) == 89)
		{
			parseString(childNode, value);
		}
	}
	_prefix_map.insert(make_pair(key, value));
}

void DBparser::replacePrefix(std::string& str)
{
	if (str[0] != '<' && str[0] != '\"' && str[0] != '?')
	{
		int sep=str.find(":");
		if (sep == -1)	return;
		std::string prefix=str.substr(0, sep+1);
		std::cout << "prefix: " << prefix << std::endl;
		if (_prefix_map.find(prefix) != _prefix_map.end())
		{
			str=_prefix_map[prefix].substr(0, _prefix_map[prefix].length() - 1) + str.substr(sep + 1 ,str.length() - sep - 1) + ">";
			std::cout << "str: " << str << std::endl;
		}
		else
		{
			std::cout << "prefix not found..." << std::endl;
			throw "Some errors are found in the SPARQL query request.";
		}
	}
}

void DBparser::parseSelectClause(pANTLR3_BASE_TREE node, SPARQLquery& query)
{
	printf("parseSelectClause\n");

	for (unsigned int j = 0; j < node->getChildCount(node); j++)
		{
				pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE) node->getChild(node, j);

				//var 199
				if (childNode->getType(childNode) == 199)
				{
					parseSelectVar(childNode, query);
				}
		}
}

void DBparser::parseSelectVar(pANTLR3_BASE_TREE node, SPARQLquery& query)
{
	printf("parseSelectVar\n");

	std::string var = "";
	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

		if (childNode->getType(childNode) == 200)
		{
			parseString(childNode,var);
			query.addOneProjection(var);
		}
	}
}

void DBparser::parseGroupPattern(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup)
{
	printf("parseGroupPattern\n");

	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

		//triples same subject 185
		if (childNode->getType(childNode) == 185)
		{
			parsePattern(childNode, patterngroup);
		}

		//optional	124
		if (childNode->getType(childNode) == 124)
		{
			parseOptional(childNode, patterngroup);
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
	}
}

void DBparser::parsePattern(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup)
{
	printf("parsePattern\n");

	std::string subject = "";
	std::string predicate = "";
	std::string object = "";
	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

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
	}
	patterngroup.addOnePattern(SPARQLquery::Pattern(SPARQLquery::Element(subject), SPARQLquery::Element(predicate), SPARQLquery::Element(object)));
}

void DBparser::parseOptional(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup)
{
	printf("parseOptional\n");
	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

		//group graph pattern 77
		if (childNode->getType(childNode) == 77)
		{
			patterngroup.addOneOptional();
			parseGroupPattern(childNode, patterngroup.getLastOptional());
		}
	}
}

void DBparser::parseUnion(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup)
{
	printf("parseUnion\n");

	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

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

void DBparser::parseFilter(pANTLR3_BASE_TREE node, SPARQLquery::PatternGroup& patterngroup)
{
	printf("parseFilter\n");

	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

		patterngroup.addOneFilterTree();
		parseFilterTree(childNode, patterngroup.getLastFilterTree());
	}
}

void DBparser::parseFilterTree(pANTLR3_BASE_TREE node, SPARQLquery::FilterTree& filter)
{
	printf("parseFilterTree\n");

	//not 192
	if (node->getType(node) == 192)	filter.type = SPARQLquery::FilterTree::Not;
	//or 125
	if (node->getType(node) == 125)	filter.type = SPARQLquery::FilterTree::Or;
	//and 8
	if (node->getType(node) == 8)		filter.type = SPARQLquery::FilterTree::And;
	//equal 62
	if (node->getType(node) == 62)		filter.type = SPARQLquery::FilterTree::Equal;
	//not equal 116
	if (node->getType(node) == 116)	filter.type = SPARQLquery::FilterTree::NotEqual;
	//less 100
	if (node->getType(node) == 100)	filter.type = SPARQLquery::FilterTree::Less;
	//less equal 101
	if (node->getType(node) == 101)	filter.type = SPARQLquery::FilterTree::LessOrEqual;
	//greater 72
	if (node->getType(node) == 72)		filter.type = SPARQLquery::FilterTree::Greater;
	//greater equal 73
	if (node->getType(node) == 73)		filter.type = SPARQLquery::FilterTree::GreaterOrEqual;

	for (unsigned int j = 0; j < node->getChildCount(node); j++)
	{
		pANTLR3_BASE_TREE childNode=(pANTLR3_BASE_TREE) node->getChild(node, j);

		//unary 190
		if (childNode->getType(childNode) == 190)
			if (j == 0)
			{
				parseString(childNode, filter.arg1, 1);
				replacePrefix(filter.arg1);
			}
			else
			{
				parseString(childNode, filter.arg2, 1);
				replacePrefix(filter.arg2);
			}
		else
			if (j == 0)
			{
				filter.parg1 = new SPARQLquery::FilterTree();
				parseFilterTree(childNode, *filter.parg1);
			}
			else
			{
				filter.parg2 = new SPARQLquery::FilterTree();
				parseFilterTree(childNode, *filter.parg2);
			}
	}
}

void DBparser::parseString(pANTLR3_BASE_TREE node, std::string& str, int depth)
{
	while (depth > 0 && node != NULL)
	{
		node = (pANTLR3_BASE_TREE) node->getChild(node, 0);
		depth--;
	}
	if (node != NULL)
		str = (const char*) node->getText(node)->chars;
	else
		throw "Some errors are found in the SPARQL query request.";
}


void DBparser::printquery(SPARQLquery& query)
{
	std::vector <std::string> &varvec = query.getProjections();
	printf("===========================================================================\n");
	printf("var is :");
	for (int i = 0; i < (int)varvec.size(); i++)
	printf("%s\t", varvec[i].c_str());
	printf("\n");
	printgrouppattern(query.getPatternGroup(), 0);
	printf("===========================================================================\n");
}

void DBparser::printgrouppattern(SPARQLquery::PatternGroup &pg, int dep)
{
	for (int j = 0; j < dep; j++)	printf("\t");	printf("{\n");
	for (int j = 0; j < dep; j++)	printf("\t");	printf("pattern:\n");
	for(int i = 0; i < pg.patterns.size(); i++)
	{
		for (int j = 0; j < dep; j++)	printf("\t");
		printf("\t%s\t%s\t%s\n", pg.patterns[i].subject.value.c_str(), pg.patterns[i].predicate.value.c_str(), pg.patterns[i].object.value.c_str());
	}

	if (pg.optionals.size() > 0)
	{
		for (int j = 0; j < dep; j++)	printf("\t");		printf("optional:\n");
		for (int i = 0; i < pg.optionals.size(); i++)
			printgrouppattern(pg.optionals[i], dep + 1);
	}

	for (int i = 0; i < pg.unions.size(); i++)
	{
		for (int j = 0; j < dep; j++)	printf("\t");	printf("union %d:\n", i + 1);
		for (int k = 0; k < pg.unions[i].size(); k++)
			printgrouppattern(pg.unions[i][k], dep + 1);
	}

	if (pg.filters.size() > 0)
	{
		for (int j = 0; j < dep; j++)	printf("\t");		printf("filter:\n");
		for (int i = 0; i < pg.filters.size(); i++)
		{
			for (int j = 0; j <= dep; j++)	printf("\t");
			printfilter(pg.filters[i]);
			printf("\n");
		}
	}

	for (int j = 0; j < dep; j++)	printf("\t");		printf("}\n");
}

void DBparser::printfilter(SPARQLquery::FilterTree &ft)
{
	printf("(");

	if (ft.type == SPARQLquery::FilterTree::Not)	printf("!");

	if (ft.parg1 == NULL)			printf("%s", ft.arg1.c_str());
	else printfilter(*ft.parg1);
	if (ft.type == SPARQLquery::FilterTree::Or)	printf("||");
	if (ft.type == SPARQLquery::FilterTree::And)	printf("&&");
	if (ft.type == SPARQLquery::FilterTree::Equal)	printf("=");
	if (ft.type == SPARQLquery::FilterTree::NotEqual)	printf("!=");
	if (ft.type == SPARQLquery::FilterTree::Less)	printf("<");
	if (ft.type == SPARQLquery::FilterTree::LessOrEqual)	printf("<=");
	if (ft.type == SPARQLquery::FilterTree::Greater)	printf(">");
	if (ft.type == SPARQLquery::FilterTree::GreaterOrEqual)	printf(">=");


	if (ft.type != SPARQLquery::FilterTree::Not)
		if (ft.parg2 == NULL)	printf("%s", ft.arg2.c_str());
		else printfilter(*ft.parg2);
	printf(")");
}

void DBparser::genQueryVec(SPARQLquery::PatternGroup &pg, SPARQLquery& query)
{
	if (pg.hasVar)
	{
		query.addBasicQuery();
		query.addQueryVarVec();

		for(int i = 0; i < pg.patterns.size(); i++)
		{
			string &sub = pg.patterns[i].subject.value;
			string &pre = pg.patterns[i].predicate.value;
			string &obj = pg.patterns[i].object.value;
			query.addTriple(Triple(sub, pre, obj));

			if (sub[0] == '?')		query.addQueryVar(sub);
			if (obj[0] == '?')		query.addQueryVar(obj);
		}
	}

	for (int i = 0; i < pg.unions.size(); i++)
		for (int j = 0; j < pg.unions[i].size(); j++)
			genQueryVec(pg.unions[i][j], query);

	for (int i = 0; i < pg.optionals.size(); i++)
		genQueryVec(pg.optionals[i], query);
}
