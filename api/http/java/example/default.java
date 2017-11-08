/*
 * JavaAPIExample.java
 *
 *  Created on: 2014-11-4
 *      Author: zengli
 */

import jgsc.GstoreConnector;

// before run this example, you must start up the GStore server at first (use command ./gserver).
public class JavaAPIExample 
{
	public static void main(String[] args)
	{
		// initialize the GStore server's IP address and port.
		GstoreConnector gc = new GstoreConnector("127.0.0.1", 9000);
		//for sparql endpoint, URL can also be used here, like freebase.gstore-pku.com:80
		//GstoreConnector gc = new GstoreConnector("tourist.gstore-pku.com", 80);

		//test download function
		//String file = gc.test_download("big.txt");
		//String file = gc.test_download("big.txt");
		//System.out.println(file.substring(file.length()-100, file.length()));
		
	    // build a new database by a RDF file.
	    // note that the relative path is related to gserver.
		gc.build("LUBM10", "data/LUBM_10.n3");
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

		//sparql = "delete where "
				//+ "{"
				//+ "<http://www.founder/121> ?predicate ?object ."
				//+ "}";				
		//sparql="select ?s where { ?s ?p ?o . }";
		//PERFORMANCE: if we use the query above(as comment), result will be very large and the time cost is large, too
		//The method to improve it is to receive a line and output/save to file at once, instead of combining all lines into a String
		//The related code is in api/http/java/src/jgsc/GstoreConnector.java
	    answer = gc.query(sparql);	    
		System.out.println(answer);
		gc.unload("LUBM10");
	}
}

