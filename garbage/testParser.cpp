/*=============================================================================
# Filename: testParser.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 19:57
# Description: 
=============================================================================*/

#include "../Util/Util.h"
#include "../Parser/DBparser.h"
#include "../Query/SPARQLquery.h"

int main()
{
#ifdef DEBUG
  Util util;
#endif
  std::cout << "start" << std::endl;
  DBparser db_parser;
  SPARQLquery sparql_query;
  std::string sparql("");
  std::string s = std::string("PREFIX dc10:  <http://purl.org/dc/elements/1.0/> ") + "PREFIX dc11:  <http://purl.org/dc/elements/1.1/> " + "SELECT ?title " + "WHERE  { { ?book dc10:title  ?title } " + "UNION { ?book dc11:title  ?title } }";
  std::string s2 = std::string("PREFIX foaf: <http://xmlns.com/foaf/0.1/> ") + "SELECT ?x1 ?x4 ?x5 WHERE {{?x1	y:hasGivenName	?x5." + "?x1	y:hasFamilyName	?x6." + "?x1	rdf:type	<wordnet_scientist_110560637>." + "?x1	y:bornIn	?x2." + "?x1	y:hasAcademicAdvisor	?x4." + "?x2	y:locatedIn	<Switzerland>." + "?x3	y:locatedIn	<Germany>." + "?x4	y:bornIn	?x3.}" + "UNION { ?book dc11:title  ?title}" + "} order by ?x5";
  std::cout << "string: " << s2 << std::endl;
  db_parser.sparqlParser(s2, sparql_query);
  sparql_query.print(cout);

  return 0;
}
