/*
 * DBparser.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#include "DBparser.h"

DBparser::DBparser()
{
  _prefix_map.clear();
}

/* input sparql query string and parse query into SPARQLquery
 * the returned string is set for log when error happen
 * */
string DBparser::sparqlParser(const string& _sparql, SPARQLquery& _sparql_query)
{
  pANTLR3_INPUT_STREAM input;
  pSparqlLexer lex;
  pANTLR3_COMMON_TOKEN_STREAM tokens;
  pSparqlParser parser;
  input = antlr3StringStreamNew((ANTLR3_UINT8*)(_sparql.c_str()), ANTLR3_ENC_UTF8, _sparql.length(), (ANTLR3_UINT8*)"QueryString");
  //input = antlr3FileStreamNew((pANTLR3_UINT8)filePath,ANTLR3_ENC_8BIT);
  lex = SparqlLexerNew(input);

  tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lex));
  parser = SparqlParserNew(tokens);

  SparqlParser_workload_return r = parser->workload(parser);
  pANTLR3_BASE_TREE root = r.tree;
  //pANTLR3_BASE_TREE treeNode;

  printNode(root);
  parseNode(root, _sparql_query, 0);
  parser->free(parser);
  tokens->free(tokens);
  lex->free(lex);
  input->close(input);
  return "";
}

/* file pointer _fp points to rdfFile
 * that was opened previously in Database::encodeRDF
 * rdfParser() will be called many times until all triples in the rdfFile is parsed
 * and after each call, a group of triples will be parsed into the vector;
 * the returned string is set for log when error happen;
 * a single line in file responds to a triple and end up with a '.'
 * tuple in a line separated by '\t'
 */
string DBparser::rdfParser(ifstream& _fin, Triple* _triple_array, int& _triple_num)
{
  memset(line_buf, 0, buf_len);
  _triple_num = 0;
  int _line_len = 0;
  while (_triple_num < DBparser::TRIPLE_NUM_PER_GROUP
         && (!_fin.eof())) {
    _fin.getline(line_buf, buf_len - 1);

    _line_len = strlen(line_buf);

    /* maybe this is an empty line at the ending of file  */
    if (_line_len < 4) {
      continue;
    }

    bool _end_with_dot = true;
    int _i_dot = 0;
    for (int i = _line_len - 1; i >= 0; i--) {
      if (line_buf[i] == '.') {
        _i_dot = i;
        break;
      }
      if (line_buf[i] == '\t') {
        _end_with_dot = false;
        break;
      }
    }

    /* check the '.' */
    if (!_end_with_dot) {
      cerr << "'.' is expected at line:" << line_buf << endl;
      cerr << " line_length = " << _line_len << endl;
      continue;
    }

    line_buf[_i_dot + 1] = '\0';
    _line_len = strlen(line_buf);

    /* find the first TAB */
    int _first_tab = -1;
    for (int i = 0; i < _line_len; i++) {
      if (line_buf[i] == '\t') {
        _first_tab = i;
        break;
      }
    }
    if (_first_tab == -1) {
      cerr << "First TAB is expected at line:" << line_buf << endl;
      continue;
    }

    /* find the second TAB */
    int _second_tab = -1;
    for (int i = _first_tab + 1; i < _line_len; i++) {
      if (line_buf[i] == '\t') {
        _second_tab = i;
        break;
      }
    }
    if (_second_tab == -1) {
      cerr << "Second TAB is expected at line:" << line_buf << endl;
    }

    /* get sub, pre, obj and add new triple */
    {
      string _line = string(line_buf);
      int _sub_size = _first_tab - 0;
      _triple_array[_triple_num].subject = _line.substr(0, _sub_size);

      int _pre_size = _second_tab - (_first_tab + 1);
      _triple_array[_triple_num].predicate = _line.substr(_first_tab + 1, _pre_size);

      /* (_line_len-1) make sure that '.' is not included */
      int _obj_size = (_line_len - 1) - (_second_tab + 1);
      _triple_array[_triple_num].object = _line.substr(_second_tab + 1, _obj_size);
    }
    _triple_num++;
  }
  return "";
}

/*
 * used in readline of FILE, avoiding new memory each time
 */
char* DBparser::line_buf = new char[100 * 1000];
int DBparser::buf_len = 100 * 1000;

int DBparser::parseString(pANTLR3_BASE_TREE node, std::string& str, int depth)
{
  const char* s = (const char*)node->getText(node)->chars;
  //std::cout<<"parseString: "<<s<<std::endl;
  if (depth == 0) {
    str = s;
  } else {
    parseString((pANTLR3_BASE_TREE)node->getChild(node, 0), str, depth - 1);
  }
  return 0;
}

int DBparser::parsePrefix(pANTLR3_BASE_TREE node, std::pair<std::string, std::string>& prefixPair)
{
  //const char* s =(const char*) node->getText(node)->chars;
  std::string key;
  std::string value;
  for (unsigned int j = 0; j < node->getChildCount(node); j++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, j);
    //prefix key string 136
    if (childNode->getType(childNode) == 136) {
      parseString(childNode, key);
    }
    //prefix value URL 89
    if (childNode->getType(childNode) == 89) {
      parseString(childNode, value);
    }
  }
  prefixPair = make_pair(key, value);
  return 0;
}

void DBparser::replacePrefix(string& str)
{
  if (str[0] != '<') {
    int sep = str.find(":");
    std::string prefix = str.substr(0, sep + 1);
    std::cout << "prefix: " << prefix << std::endl;
    if (_prefix_map.find(prefix) != _prefix_map.end()) {
      str = _prefix_map[prefix].substr(0, _prefix_map[prefix].length() - 1) + str.substr(sep + 1, str.length() - sep - 1) + ">";
      std::cout << "str: " << str << std::endl;
    } else {
      std::cout << "prefix not found..." << std::endl;
    }
  }
}

int DBparser::parseTriple(pANTLR3_BASE_TREE node, Triple& triple)
{
  //const char* s =(const char*) node->getText(node)->chars;
  std::string subject = "";
  std::string predicate = "";
  std::string object = "";
  for (unsigned int j = 0; j < node->getChildCount(node); j++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, j);
    //subject 177
    if (childNode->getType(childNode) == 177) {
      parseString(childNode, subject, 1);
      replacePrefix(subject);
    }
    //predicate 142
    if (childNode->getType(childNode) == 142) {
      parseString(childNode, predicate, 4);
      replacePrefix(predicate);
    }
    //object 119
    if (childNode->getType(childNode) == 119) {
      parseString(childNode, object, 1);
      replacePrefix(object);
    }
  }
  triple = Triple(subject, predicate, object);
  std::cout << "Triple: \n\ts|" << subject << "|\n\tp|" << predicate << "|\n\to|" << object << "|" << std::endl;
  return 0;
}

int DBparser::parseBasicQuery(pANTLR3_BASE_TREE node, BasicQuery& basicQuery)
{
  //const char* s =(const char*) node->getText(node)->chars;
  Triple triple;
  for (unsigned int j = 0; j < node->getChildCount(node); j++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, j);
    //basicQuery 185
    std::cout << "Child type: " << childNode->getType(childNode) << endl;
    if (childNode->getType(childNode) == 185) {
      parseTriple(childNode, triple);
      basicQuery.addTriple(triple);
    }
    if (childNode->getType(childNode) == 195) {
      //Union part here!!
      //parseUnion(childNode,U);
      //basicQuery.addTriple(triple);
    }
  }
  return 0;
}

int DBparser::parseVar(pANTLR3_BASE_TREE node, SPARQLquery& query)
{
  //const char* s =(const char*) node->getText(node)->chars;
  std::string var = "";
  for (unsigned int j = 0; j < node->getChildCount(node); j++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, j);
    //var 200
    if (childNode->getType(childNode) == 200) {
      parseString(childNode, var, 0);
      query.addQueryVar(var);
    }
  }
  return 0;
}

int DBparser::parseNode(pANTLR3_BASE_TREE node, SPARQLquery& query, int depth)
{
  const char* s = (const char*)node->getText(node)->chars;
  ANTLR3_UINT32 treeType = node->getType(node);

  for (int i = 0; i < depth; i++) {
    printf("    ");
  }
  printf("%d: %s\n", treeType, s);

  for (unsigned int j = 0; j < node->getChildCount(node); j++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, j);
    int childNodeType = childNode->getType(childNode);
    switch (childNodeType) {
    //prefix
    case 199: {
      parseVar(childNode, query);
      break;
    }
    //var
    case 143: {
      std::pair<std::string, std::string> prefixPair;
      parsePrefix(childNode, prefixPair);
      _prefix_map.insert(prefixPair);
      break;
    }
    //BasicQuery
    case 77: {
      BasicQuery* basicQuery = new BasicQuery();
      parseBasicQuery(childNode, *basicQuery);
      query.addBasicQuery(basicQuery);
      break;
    }
    default:
      parseNode(childNode, query, depth + 1);
    }
  }
  return 0;
}

void DBparser::printNode(pANTLR3_BASE_TREE node, int depth)
{
  const char* s = (const char*)node->getText(node)->chars;
  ANTLR3_UINT32 treeType = node->getType(node);

  for (int i = 0; i < depth; i++) {
    printf("    ");
  }
  printf("%d: %s\n", treeType, s);
  for (unsigned int j = 0; j < node->getChildCount(node); j++) {
    pANTLR3_BASE_TREE childNode = (pANTLR3_BASE_TREE)node->getChild(node, j);
    //int childNodeType = childNode->getType(childNode);
    printNode(childNode, depth + 1);
  }
}
