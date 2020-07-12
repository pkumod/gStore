/*=============================================================================
# Filename: CppAPIExample.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-21 21:32
# Description: originally written by hanshuo, modified by zengli
=============================================================================*/

#include "GstoreConnector.h"
#include <string>
#include <iostream>

// before run this example, you must start up the GStore server at first (use command ./gserver).
int main(int argc, char* argv[])
{
  // initialize the GStore server's IP address and port.
  GstoreConnector gc("127.0.0.1", 3305);

  // build a new database by a RDF file.
  // note that the relative path is related to gserver.
  gc.build("LUBM10", "data/lubm/lubm.nt");
  gc.load("LUBM10");

  // then you can execute SPARQL query on this database.
  std::string sparql = "select ?x where \
                         { \
                         ?x    <rdf:type>    <ub:UndergraduateStudent>. \
                         ?y    <ub:name> <Course1>. \
                         ?x    <ub:takesCourse>  ?y. \
                         ?z    <ub:teacherOf>    ?y. \
                         ?z    <ub:name> <FullProfessor1>. \
                         ?z    <ub:worksFor>    ?w. \
                         ?w    <ub:name>    <Department0>. \
                         }";
  std::string answer = gc.query(sparql);
  std::cout << answer << std::endl;

  // unload this database.
  gc.unload("LUBM10");

  // also, you can load some exist database directly and then query.
  gc.load("LUBM10");
  answer = gc.query(sparql);
  std::cout << answer << std::endl;
  gc.unload("LUBM10");

  return 0;
}
