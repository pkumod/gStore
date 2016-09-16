/*
 * CppAPIExample.cpp
 *
 *  Created on: 2014-11-4
 *      Author: hanshuo
 */

#include"GstoreConnector.h"
#include<string>
#include<iostream>

// before run this example, you must start up the GStore server at first (use command ./gserver).
int main(int argc, char * argv[])
{
    // initialize the GStore server's IP address and port.
    GstoreConnector gc("127.0.0.1", 3305);

    // build a new database by a RDF file.
    // note that the relative path is related to gserver.
    gc.build("db_LUBM10", "example/rdf_triple/LUBM_10_GStore.n3");

    // then you can execute SPARQL query on this database.
    std::string sparql = "select ?x where \
                         { \
                         ?x    rdf:type    <ub:UndergraduateStudent>. \
                         ?y    ub:name <Course1>. \
                         ?x    ub:takesCourse  ?y. \
                         ?z    ub:teacherOf    ?y. \
                         ?z    ub:name <FullProfessor1>. \
                         ?z    ub:worksFor    ?w. \
                         ?w    ub:name    <Department0>. \
                         }";
    std::string answer = gc.query(sparql);
    std::cout << answer << std::endl;

    // unload this database.
    gc.unload("db_LUBM10");

    // also, you can load some exist database directly and then query.
    gc.load("db_LUBM10");
    answer = gc.query(sparql);
    std::cout << answer << std::endl;

    return 0;
}


