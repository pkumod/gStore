/*=============================================================================
# Filename: QueryParser.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-13
# Description: implement functions in QueryParser.h
=============================================================================*/

#include "QueryParser.h"

using namespace std;

void QueryParser::SPARQLParse(const string& query, QueryTree& querytree)
{
  //uncompress before use
    printf("[Info]	test step 1 \n");
    fflush(stdout);
  dfa34_Table_uncompress();
  printf("[Info]	test step 2 \n");
  fflush(stdout);
  pANTLR3_INPUT_STREAM input = antlr3StringStreamNew((ANTLR3_UINT8*)(query.c_str()), ANTLR3_ENC_UTF8, query.length(), (ANTLR3_UINT8*)"QueryString");
  //input = antlr3FileStreamNew((pANTLR3_UINT8)filePath,ANTLR3_ENC_8BIT);
  printf("[Info]	test step 3 \n");
  fflush(stdout);
  pSparqlLexer lex = SparqlLexerNew(input);
  printf("[Info]	test step 4 \n");
  fflush(stdout);
  pANTLR3_COMMON_TOKEN_STREAM tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
  printf("[Info]	test step 5 \n");
  fflush(stdout);
  pSparqlParser parser = SparqlParserNew(tokens);
  printf("[Info]	test step 6 \n");
  fflush(stdout);

  SparqlParser_workload_return workload_ret = parser->workload(parser);
  printf("[Info]	test step 7 \n");
  fflush(stdout);
  pANTLR3_BASE_TREE root = workload_ret.tree;

  if (printNode(root) > 0)
  {

      printf("[ERROR]	Some errors are found in the SPARQL query request.");
      fflush(stdout);
      throw "[ERROR]	Some errors are found in the SPARQL query request.";
  }

  parseWorkload(root, querytree);

  querytree.print();

  parser->free(parser);
  tokens->free(tokens);
  lex->free(lex);
  input->close(input);
}

int QueryParser::printNode(pANTLR3_BASE_TREE node, int dep)
{
  const char* s = (const char*)node->getText(node)->chars;
  ANTLR3_UINT32 treeType = node->getType(node);

  int hasErrorNode = (treeType == 0 ? 1 : 0);

  for (int t = 0; t < dep; t++)
    printf("\t");
  printf("%d: %s\n", treeType, s);

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);
    hasErrorNode += printNode(childNode, dep + 1);
  }

  return hasErrorNode;
}

void QueryParser::parseWorkload(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseWorkload\n");

  pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, 0);

  //query 145
  if (childNode->getType(childNode) == 145) {
    parseQuery(childNode, querytree);
  }
  //update 196
  else if (childNode->getType(childNode) == 196) {
    parseUpdate(childNode, querytree);
  }
}

void QueryParser::parseQuery(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseQuery\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //prologue	144
    if (childNode->getType(childNode) == 144) {
      parsePrologue(childNode);
    }
    //select 155
    else if (childNode->getType(childNode) == 155) {
      querytree.setQueryForm(QueryTree::Select_Query);
      parseQuery(childNode, querytree);
    }
    //ask 13
    else if (childNode->getType(childNode) == 13) {
      querytree.setQueryForm(QueryTree::Ask_Query);
      querytree.setProjectionAsterisk();
      parseQuery(childNode, querytree);
    }
    //select clause 156
    else if (childNode->getType(childNode) == 156) {
      parseSelectClause(childNode, querytree);
    }
    //group graph pattern 77
    else if (childNode->getType(childNode) == 77) {
      parseGroupPattern(childNode, querytree.getGroupPattern());
    }
    //group by 75
    else if (childNode->getType(childNode) == 75) {
      parseGroupBy(childNode, querytree);
    }
    //order by 127
    else if (childNode->getType(childNode) == 127) {
      parseOrderBy(childNode, querytree);
    }
    //offset 120	limit 102
    else if (childNode->getType(childNode) == 120 || childNode->getType(childNode) == 102) {
      pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

      //integer 83
      if (gchildNode->getType(gchildNode) == 83) {
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
    } else
      parseQuery(childNode, querytree);
  }
}

void QueryParser::parsePrologue(pANTLR3_BASE_TREE node)
{
  printf("parsePrologue\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //prefix 143
    if (childNode->getType(childNode) == 143)
      parsePrefix(childNode);
  }
}

void QueryParser::parsePrefix(pANTLR3_BASE_TREE node)
{
  printf("parsePrefix\n");

  string key, value;

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //prefix namespace 136
    if (childNode->getType(childNode) == 136)
      parseString(childNode, key, 0);

    //prefix IRI 89
    if (childNode->getType(childNode) == 89)
      parseString(childNode, value, 0);
  }

  this->prefix_map[key] = value;
}

void QueryParser::replacePrefix(string& str)
{
  if (str[0] != '<' && str[0] != '\"' && str[0] != '?') {
    int sep = str.find(":");
    if (sep == -1)
      return;

    string prefix = str.substr(0, sep + 1);

    //blank node
    if (prefix == "_:")
      return;

    printf("prefix = %s\n", prefix.c_str());
    if (this->prefix_map.count(prefix) != 0) {
      str = this->prefix_map[prefix].substr(0, this->prefix_map[prefix].length() - 1) + str.substr(sep + 1, str.length() - sep - 1) + ">";
      printf("str = %s\n", str.c_str());
    } else {
      printf("prefix not found...\n");
      throw "[ERROR]	Prefix is not found, please define it before use.";
    }
  }
}

void QueryParser::parseSelectClause(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseSelectClause\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //distinct 52
    if (childNode->getType(childNode) == 52)
      querytree.setProjectionModifier(QueryTree::Modifier_Distinct);

    //var 199
    if (childNode->getType(childNode) == 199)
      parseSelectVar(childNode, querytree);

    //as 11
    if (childNode->getType(childNode) == 11)
      parseSelectAggregateFunction(childNode, querytree);

    //asterisk 14
    if (childNode->getType(childNode) == 14)
      querytree.setProjectionAsterisk();
  }
}

void QueryParser::parseSelectVar(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseSelectVar\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    if (childNode->getType(childNode) == 200) {
      querytree.addProjectionVar();
      QueryTree::ProjectionVar& proj_var = querytree.getLastProjectionVar();
      proj_var.aggregate_type = QueryTree::ProjectionVar::None_type;

      parseString(childNode, proj_var.var, 0);
    }
  }
}

void QueryParser::parseSelectAggregateFunction(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseSelectAggregateFunction\n");

  //AS
  //UNARY
  //COUNT
  //DISTINCT
  //UNARY
  //var 200
  //asterisk 14
  //var 200

  pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, 0);

  //unary 190
  while (childNode->getType(childNode) == 190)
    childNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);
  //count 39
  if (childNode->getType(childNode) != 39)
    throw "[ERROR]	The supported aggregate function now is COUNT only.";

  bool distinct = false;
  pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

  //distinct 52
  if (gchildNode->getType(gchildNode) == 52) {
    distinct = true;
    gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 1);
  }

  //unary 190
  while (gchildNode->getType(gchildNode) == 190)
    gchildNode = (pANTLR3_BASE_TREE)gchildNode->getChild(gchildNode, 0);
  if (gchildNode->getType(gchildNode) != 200 && gchildNode->getType(gchildNode) != 14)
    throw "[ERROR]	The aggregate function COUNT can accept only one var or *.";

  querytree.addProjectionVar();
  QueryTree::ProjectionVar& proj_var = querytree.getLastProjectionVar();
  proj_var.aggregate_type = QueryTree::ProjectionVar::Count_type;
  proj_var.distinct = distinct;

  if (gchildNode->getType(gchildNode) == 200) {
    parseString(gchildNode, proj_var.aggregate_var, 0);
  }
  if (gchildNode->getType(gchildNode) == 14) {
    parseString(gchildNode, proj_var.aggregate_var, 0); //for convenience, set aggregate_var *
    querytree.setProjectionAsterisk();
  }

  childNode = (pANTLR3_BASE_TREE)node->getChild(node, 1);
  parseString(childNode, proj_var.var, 0);
}

void QueryParser::parseGroupPattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  printf("parseGroupPattern\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //triples same subject 185
    if (childNode->getType(childNode) == 185) {
      parsePattern(childNode, group_pattern);
    }

    //optional	124		minus	108
    if (childNode->getType(childNode) == 124 || childNode->getType(childNode) == 108) {
      parseOptionalOrMinus(childNode, group_pattern);
    }

    //union  195
    if (childNode->getType(childNode) == 195) {
      group_pattern.addOneGroupUnion();
      parseUnion(childNode, group_pattern);
    }

    //filter 67
    if (childNode->getType(childNode) == 67) {
      parseFilter(childNode, group_pattern);
    }

    //bind 17
    if (childNode->getType(childNode) == 17) {
      parseBind(childNode, group_pattern);
    }

    //group graph pattern 77
    //redundant {}
    if (childNode->getType(childNode) == 77) {
      parseGroupPattern(childNode, group_pattern);
    }
  }
}

void QueryParser::parsePattern(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  printf("parsePattern\n");

  string subject, predicate, object;
  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //subject 177
    if (childNode->getType(childNode) == 177) {
      parseString(childNode, subject, 1);
      replacePrefix(subject);
    }

    //predicate 142
    if (childNode->getType(childNode) == 142) {
      pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

      //var 200
      if (gchildNode->getType(gchildNode) == 200) {
        parseString(childNode, predicate, 1);
      } else {
        parseString(childNode, predicate, 4);
        if (predicate == "a")
          predicate = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>";
      }
      replacePrefix(predicate);
    }

    //object 119
    if (childNode->getType(childNode) == 119) {
      parseString(childNode, object, 1);
      replacePrefix(object);
    }

    if (i != 0 && i % 2 == 0) //triples same subject
    {
      group_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(QueryTree::GroupPattern::Pattern::Element(subject),
                                                                   QueryTree::GroupPattern::Pattern::Element(predicate),
                                                                   QueryTree::GroupPattern::Pattern::Element(object)));

      //scope of filter
      for (int j = (int)group_pattern.sub_group_pattern.size() - 1; j > 0; j--)
        if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type && group_pattern.sub_group_pattern[j - 1].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type) {
          QueryTree::GroupPattern::SubGroupPattern tmp(group_pattern.sub_group_pattern[j - 1]);
          group_pattern.sub_group_pattern[j - 1] = group_pattern.sub_group_pattern[j];
          group_pattern.sub_group_pattern[j] = tmp;
        } else
          break;
    }
  }
}

void QueryParser::parseOptionalOrMinus(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  //optional 124
  if (node->getType(node) == 124)
    printf("parseOptional\n");
  //minus	108
  else if (node->getType(node) == 108)
    printf("parseMinus\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //group graph pattern 77
    if (childNode->getType(childNode) == 77) {
      if (node->getType(node) == 124)
        group_pattern.addOneOptional(QueryTree::GroupPattern::SubGroupPattern::Optional_type);
      else if (node->getType(node) == 108)
        group_pattern.addOneOptional(QueryTree::GroupPattern::SubGroupPattern::Minus_type);

      parseGroupPattern(childNode, group_pattern.getLastOptional());
    }
  }
}

void QueryParser::parseUnion(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  printf("parseUnion\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //group graph pattern 77
    if (childNode->getType(childNode) == 77) {
      group_pattern.addOneUnion();
      parseGroupPattern(childNode, group_pattern.getLastUnion());
    }

    //union  195
    if (childNode->getType(childNode) == 195) {
      parseUnion(childNode, group_pattern);
    }
  }
}

void QueryParser::parseFilter(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  printf("parseFilter\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //unary 190
    while (childNode->getType(childNode) == 190)
      childNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

    group_pattern.addOneFilter();
    parseFilterTree(childNode, group_pattern.getLastFilter().root);
  }
}

void QueryParser::parseFilterTree(pANTLR3_BASE_TREE node, QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter)
{
  printf("parseFilterTree\n");

  switch (node->getType(node)) {
  //! 192
  case 192:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Not_type;
    break;
  //not 115
  case 115:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Not_type;
    break;
  //or 125
  case 125:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Or_type;
    break;
  //and 8
  case 8:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::And_type;
    break;
  //equal 62
  case 62:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Equal_type;
    break;
  //not equal 116
  case 116:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::NotEqual_type;
    break;
  //less 100
  case 100:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Less_type;
    break;
  //less equal 101
  case 101:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::LessOrEqual_type;
    break;
  //greater 72
  case 72:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Greater_type;
    break;
  //greater equal 73
  case 73:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::GreaterOrEqual_type;
    break;

  //regex 150
  case 150:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_regex_type;
    break;
  //str 167
  case 167:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_str_type;
    break;
  //isIRI 92
  case 92:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isiri_type;
    break;
  //isURI 95
  case 95:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isuri_type;
    break;
  //isLiteral 93
  case 93:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isliteral_type;
    break;
  //isNumeric 94
  case 94:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_isnumeric_type;
    break;
  //lang 96
  case 96:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_lang_type;
    break;
  //langMatches 97
  case 97:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_langmatches_type;
    break;
  //bound 23
  case 23:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_bound_type;
    break;
  //in 81
  case 81:
    filter.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_in_type;
    break;

  default:
    return;
  }

  //in the "NOT IN" case, in, var and expression list is on the same layer.
  //not 115
  if (node->getType(node) == 115) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, 0);

    //in 81
    if (childNode->getType(childNode) == 81) {
      filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());
      filter.child[0].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
      filter.child[0].node.oper_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::Builtin_in_type;
      parseVarInExpressionList(node, filter.child[0].node);

      return;
    }
  }

  //in 81
  if (node->getType(node) == 81) {
    parseVarInExpressionList(node, filter);

    return;
  }

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //unary 190
    while (childNode->getType(childNode) == 190) {
      pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

      if (gchildNode->getChildCount(gchildNode) != 0)
        childNode = gchildNode;
      else
        break;
    }

    filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());

    //unary 190
    if (childNode->getType(childNode) == 190) {
      filter.child[i].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
      parseString(childNode, filter.child[i].str, 1);
      replacePrefix(filter.child[i].str);
    } else if (childNode->getChildCount(childNode) == 0) {
      filter.child[i].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
      parseString(childNode, filter.child[i].str, 0);
      replacePrefix(filter.child[i].str);
    } else {
      filter.child[i].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
      parseFilterTree(childNode, filter.child[i].node);
    }
  }
}

void QueryParser::parseVarInExpressionList(pANTLR3_BASE_TREE node, QueryTree::GroupPattern::FilterTree::FilterTreeNode& filter)
{
  printf("parseVarInExpressionList\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //in 81
    if (childNode->getType(childNode) == 81)
      continue;

    //unary 190
    if (childNode->getType(childNode) == 190) {
      //unary 190
      while (childNode->getType(childNode) == 190) {
        pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

        if (gchildNode->getChildCount(gchildNode) != 0)
          childNode = gchildNode;
        else
          break;
      }

      int last = filter.child.size();
      filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());

      //unary 190
      if (childNode->getType(childNode) == 190) {
        filter.child[last].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
        parseString(childNode, filter.child[last].str, 1);
        replacePrefix(filter.child[last].str);
      } else if (childNode->getChildCount(childNode) == 0) {
        filter.child[last].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
        parseString(childNode, filter.child[last].str, 0);
        replacePrefix(filter.child[last].str);
      } else {
        filter.child[last].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
        parseFilterTree(childNode, filter.child[last].node);
      }
    }

    //expression list 65
    if (childNode->getType(childNode) == 65) {
      for (unsigned int j = 0; j < childNode->getChildCount(childNode); j++) {
        pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, j);

        //unary 190
        while (gchildNode->getType(gchildNode) == 190) {
          pANTLR3_BASE_TREE ggchildNode = (pANTLR3_BASE_TREE)gchildNode->getChild(gchildNode, 0);

          if (ggchildNode->getChildCount(ggchildNode) != 0)
            gchildNode = ggchildNode;
          else
            break;
        }

        int last = filter.child.size();
        filter.child.push_back(QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild());

        //unary 190
        if (gchildNode->getType(gchildNode) == 190) {
          filter.child[last].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
          parseString(gchildNode, filter.child[last].str, 1);
          replacePrefix(filter.child[last].str);
        } else if (gchildNode->getChildCount(gchildNode) == 0) {
          filter.child[last].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::String_type;
          parseString(gchildNode, filter.child[last].str, 0);
          replacePrefix(filter.child[last].str);
        } else {
          filter.child[last].node_type = QueryTree::GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild::Tree_type;
          parseFilterTree(gchildNode, filter.child[last].node);
        }
      }
    }
  }
}

void QueryParser::parseBind(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  printf("parseBind\n");

  string str, var;

  pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, 0);

  //unary 190
  while (childNode->getType(childNode) == 190) {
    pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

    if (gchildNode->getChildCount(gchildNode) != 0)
      childNode = gchildNode;
    else
      break;
  }

  //unary 190
  if (childNode->getType(childNode) == 190) {
    parseString(childNode, str, 1);
    replacePrefix(str);
  } else if (childNode->getChildCount(childNode) == 0) {
    parseString(childNode, str, 0);
    replacePrefix(str);
  } else {
    throw "[ERROR]	The BIND operator can't assign an expression to a var.";
  }

  childNode = (pANTLR3_BASE_TREE)node->getChild(node, 1);
  //as 11
  if (childNode->getType(childNode) == 11) {
    parseString(childNode, var, 1);
  }

  group_pattern.addOneBind();
  group_pattern.getLastBind() = QueryTree::GroupPattern::Bind(str, var);
}

void QueryParser::parseGroupBy(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseGroupBy\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //var 200
    if (childNode->getType(childNode) == 200) {
      string var;
      parseString(childNode, var, 0);

      querytree.addGroupByVar(var);
    } else
      throw "[ERROR]	The supported GROUP BY key is var only.";
  }
}

void QueryParser::parseOrderBy(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseOrderBy\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //order by condition
    if (childNode->getType(childNode) == 128) {
      string var;
      bool desending = false;

      for (unsigned int j = 0; j < childNode->getChildCount(childNode); j++) {
        pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, j);

        //unary 190
        while (gchildNode->getType(gchildNode) == 190)
          gchildNode = (pANTLR3_BASE_TREE)gchildNode->getChild(gchildNode, 0);

        //var 200
        if (gchildNode->getType(gchildNode) == 200)
          parseString(gchildNode, var, 0);

        //asend 12
        if (gchildNode->getType(gchildNode) == 12)
          desending = false;

        //desend 49
        if (gchildNode->getType(gchildNode) == 49)
          desending = true;
      }

      if (var.length() > 0)
        querytree.addOrderVar(var, desending);
    }
  }
}

void QueryParser::parseString(pANTLR3_BASE_TREE node, string& str, int dep)
{
  if (dep == 0) {
    str = (const char*)node->getText(node)->chars;
    return;
  }

  while (dep > 1 && node != NULL) {
    node = (pANTLR3_BASE_TREE)node->getChild(node, 0);
    dep--;
  }

  if (node == NULL || node->getChildCount(node) == 0)
    throw "[ERROR]	Some errors are found in the SPARQL query request.";
  else {
    str = "";
    for (unsigned int i = 0; i < node->getChildCount(node); i++) {
      pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

      //var 200
      //string literal 170(single quotation marks)	171(double quotation marks)
      //IRI 89
      //PNAME_LN 135
      //custom language 98

      //'' 170
      //"" 171
      //'''''' 172
      //"""""" 173

      string substr = (const char*)childNode->getText(childNode)->chars;
      if (childNode->getType(childNode) == 170)
        substr = "\"" + substr.substr(1, substr.length() - 2) + "\"";
      if (childNode->getType(childNode) == 172)
        substr = "\"" + substr.substr(3, substr.length() - 6) + "\"";
      if (childNode->getType(childNode) == 173)
        substr = "\"" + substr.substr(3, substr.length() - 6) + "\"";

      if (i > 0) {
        pANTLR3_BASE_TREE preChildNode = (pANTLR3_BASE_TREE)node->getChild(node, i - 1);

        //xsd:*
        //reference	149
        if (preChildNode->getType(preChildNode) == 149)
          replacePrefix(substr);
      }

      str += substr;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void QueryParser::parseUpdate(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseUpdate\n");

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //prologue	144
    if (childNode->getType(childNode) == 144) {
      parsePrologue(childNode);
    }
    //insert 82
    else if (childNode->getType(childNode) == 82) {
      //INSERT
      //DATA
      //TRIPLES_TEMPLATE

      querytree.setUpdateType(QueryTree::Insert_Data);

      pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 1);

      //triples template 186
      if (gchildNode->getType(gchildNode) == 186)
        parseTripleTemplate(gchildNode, querytree.getInsertPatterns());
    }
    //delete 48
    else if (childNode->getType(childNode) == 48 && childNode->getChildCount(childNode) > 0) {
      //DELETE
      //DELETE
      //DATA or WHERE
      //TRIPLES_TEMPLATE

      pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);

      //data 41
      if (gchildNode->getType(gchildNode) == 41) {
        querytree.setUpdateType(QueryTree::Delete_Data);

        gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 1);
        //triples template 186
        if (gchildNode->getType(gchildNode) == 186)
          parseTripleTemplate(gchildNode, querytree.getDeletePatterns());
      }
      //where 203
      else if (gchildNode->getType(gchildNode) == 203) {
        querytree.setUpdateType(QueryTree::Delete_Where);

        gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 1);
        //triples template 186
        if (gchildNode->getType(gchildNode) == 186) {
          parseTripleTemplate(gchildNode, querytree.getDeletePatterns());
          querytree.getGroupPattern() = querytree.getDeletePatterns();
        }
      }
    }
    //modify 110
    else if (childNode->getType(childNode) == 110) {
      parseModify(childNode, querytree);
    }
  }
}

void QueryParser::parseTripleTemplate(pANTLR3_BASE_TREE node, QueryTree::GroupPattern& group_pattern)
{
  printf("parseTripleTemplate\n");

  //TRIPLES_TEMPLATE
  //TRIPLES_SAME_SUBJECT
  //SUBJECT
  //...
  //PREDICATE
  //...
  //OBJECT
  //...

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //triples same subject 185
    if (childNode->getType(childNode) == 185) {
      string subject, predicate, object;

      for (unsigned int j = 0; j < childNode->getChildCount(childNode); j++) {
        pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, j);

        //subject 177
        if (gchildNode->getType(gchildNode) == 177) {
          parseString(gchildNode, subject, 1);
          replacePrefix(subject);
        }

        //predicate 142
        if (gchildNode->getType(gchildNode) == 142) {
          parseString(gchildNode, predicate, 1);
          if (predicate == "a")
            predicate = "<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>";
          replacePrefix(predicate);
        }

        //object 119
        if (gchildNode->getType(gchildNode) == 119) {
          parseString(gchildNode, object, 1);
          replacePrefix(object);
        }

        if (j != 0 && j % 2 == 0) //triples same subject
        {
          group_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(QueryTree::GroupPattern::Pattern::Element(subject),
                                                                       QueryTree::GroupPattern::Pattern::Element(predicate),
                                                                       QueryTree::GroupPattern::Pattern::Element(object)));
        }
      }
    }
  }
}

void QueryParser::parseModify(pANTLR3_BASE_TREE node, QueryTree& querytree)
{
  printf("parseModify\n");

  //DELETE
  //TRIPLES_TEMPLATE
  //TRIPLES_SAME_SUBJECT
  //INSERT
  //TRIPLES_TEMPLATE
  //TRIPLES_SAME_SUBJECT
  //WHERE
  //GROUP_GRAPH_PATTERN

  for (unsigned int i = 0; i < node->getChildCount(node); i++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, i);

    //delete 48
    if (childNode->getType(childNode) == 48) {
      querytree.setUpdateType(QueryTree::Delete_Clause);
    }
    //insert 82
    else if (childNode->getType(childNode) == 82) {
      if (querytree.getUpdateType() == QueryTree::Not_Update)
        querytree.setUpdateType(QueryTree::Insert_Clause);
      else if (querytree.getUpdateType() == QueryTree::Delete_Clause)
        querytree.setUpdateType(QueryTree::Modify_Clause);
    }
    //triples template 186
    else if (childNode->getType(childNode) == 186) {
      if (querytree.getUpdateType() == QueryTree::Delete_Clause)
        parseTripleTemplate(childNode, querytree.getDeletePatterns());
      else if (querytree.getUpdateType() == QueryTree::Insert_Clause || querytree.getUpdateType() == QueryTree::Modify_Clause)
        parseTripleTemplate(childNode, querytree.getInsertPatterns());
    }
    //where 203
    else if (childNode->getType(childNode) == 203) {
      //WHERE
      //GROUP_GRAPH_PATTERN

      pANTLR3_BASE_TREE gchildNode = (pANTLR3_BASE_TREE)childNode->getChild(childNode, 0);
      //group graph pattern 77
      if (gchildNode->getType(gchildNode) == 77)
        parseGroupPattern(gchildNode, querytree.getGroupPattern());
    }
  }
}
