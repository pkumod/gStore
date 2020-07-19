/*
 * JavaAPIExample.java
 *
 *  Created on: 2014-11-4
 *      Author: hanshuo
 */

import jgsc.GstoreConnector;

// before run this example, you must start up the GStore server at first (use command ./gserver).
public class JavaAPIExample {
    public static void main(String[] args) {
        // initialize the GStore server's IP address and port.
        GstoreConnector gc = new GstoreConnector("127.0.0.1", 3305);

        // build a new database by a RDF file.
        // note that the relative path is related to gserver.
        gc.build("LUBM10", "data/lubm/lubm.nt");
        gc.load("LUBM10");

        // then you can execute SPARQL query on this database.
        String sparql = "select ?x where "
                        + "{"
                        + "?x    <rdf:type>    <ub:UndergraduateStudent>. "
                        + "?y    <ub:name> <Course1>. "
                        + "?x    <ub:takesCourse>  ?y. "
                        + "?z    <ub:teacherOf>    ?y. "
                        + "?z    <ub:name> <FullProfessor1>. "
                        + "?z    <ub:worksFor>    ?w. "
                        + "?w    <ub:name>    <Department0>. "
                        + "}";
        String answer = gc.query(sparql);
        System.out.println(answer);

        // unload this database.
        gc.unload("LUBM10");

        // also, you can load some exist database directly and then query.
        gc.load("LUBM10");
        answer = gc.query(sparql);
        System.out.println(answer);
        gc.unload("LUBM10");
    }
}
