/*
# Filename: POSTexample.java
# Author: suxunbin
# Last Modified: 2019-5-16 21:30
# Description: a simple POST-example of java API
*/

import jgsc.GstoreConnector;

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
public class POSTexample{
	public static void main(String[] args)
	{
        String IP = "127.0.0.1";
		int Port = 9000;
		String httpType = "ghttp";
        String username = "root";
        String password = "123456";
		String sparql = "select ?x where"
			+"{"
			+"?x  <rdf:type> <ub:UndergraduateStudent>."
			+"?y    <ub:name> <Course1>."
			+"?x    <ub:takesCourse>  ?y."
			+"?z   <ub:teacherOf>    ?y."
			+"?z    <ub:name> <FullProfessor1>."
			+"?z    <ub:worksFor>    ?w."
			+"?w    <ub:name>    <Department0>."
			+"}";	
        String filename = "res.txt";

		// start a gc with given IP, Port, username and password
		GstoreConnector gc = new GstoreConnector(IP, Port, httpType, username, password);

		// build a database with a RDF graph
		String res = gc.build("lubm", "data/lubm/lubm.nt", "POST");
		System.out.println(res);

		// load the database 
        res = gc.load("lubm", "0", "POST");
        System.out.println(res);

		// show all users
        res = gc.showuser("POST");
        System.out.println(res);

		// query
        res = gc.query("lubm", "json", sparql, "POST");
        System.out.println(res);

		// query and save the result in a file
        gc.fquery("lubm", "json", sparql, filename, "POST");

        // save the database if you have changed the database
		res = gc.checkpoint("lubm", "POST");
		System.out.println(res);

		// show information of the database
		res = gc.monitor("lubm", "POST");
		System.out.println(res);

		// show all databases
		res = gc.show("POST");
		System.out.println(res);


		// export the database
		res = gc.exportDB("lubm", "export/lubm/post", "POST");
		System.out.println(res);


		// unload the database
		res = gc.unload("lubm", "POST");
		System.out.println(res);

		// drop the database
		res = gc.drop("lubm", false, "POST"); //delete the database directly
		//res = gc.drop("lubm", true, "POST"); //leave a backup
		System.out.println(res);

	}
}

