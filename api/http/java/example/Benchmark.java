/*
 * JavaAPIExample.java
 *
 *  Created on: 2014-11-4
 *      Author: zengli
 */

import jgsc.GstoreConnector;

class MyThread extends Thread  {
	private int num = 0;
	private GstoreConnector gc;
	private String sparql;
	public MyThread(int _num, GstoreConnector _gc, String _sparql) {
		num = _num;
		gc = _gc;
		sparql = _sparql;
	}
	@Override
	public void run() {
		Thread t = Thread.currentThread();
		System.out.println(t.getName()+" "+t.getId());
		System.out.println("Thread: "+num);
		String answer = gc.query(sparql);
		System.out.println(answer);
		System.out.println("Thread "+t.getName()+" ends!");
	}
}

// before run this example, you must start up the GStore server at first (use command ./gserver).
public class Benchmark
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
		gc.load("lubm");
		//gc.load("dbpedia");
		
	String[] spq = new String[6];
	spq[0] = "select ?x where { ?x <ub:name> <FullProfessor0> . }";
	spq[1] = "select distinct ?x where { ?x      <rdf:type>      <ub:GraduateStudent>. ?y      <rdf:type>      <ub:University>. ?z      <rdf:type>      <ub:Department>. ?x      <ub:memberOf>   ?z. ?z      <ub:subOrganizationOf>  ?y. ?x      <ub:undergraduateDegreeFrom>    ?y. }";
	spq[2] = "select distinct ?x where { ?x      <rdf:type>      <ub:Course>. ?x      <ub:name>       ?y. }";
	spq[3] = "select ?x where { ?x    <rdf:type>    <ub:UndergraduateStudent>. ?y    <ub:name> <Course1>. ?x    <ub:takesCourse>  ?y. ?z    <ub:teacherOf>    ?y. ?z    <ub:name> <FullProfessor1>. ?z    <ub:worksFor>    ?w. ?w    <ub:name>    <Department0>. }";
	spq[4] = "select distinct ?x where { ?x    <rdf:type>    <ub:UndergraduateStudent>. }";
	spq[5] = "select ?s ?o where { ?s ?p ?o . }";
	int tnum = 6;
	//tnum = 12000;
	MyThread[] qt = new MyThread[tnum];
	for(int i = 0; i < tnum; ++i)
	{
		qt[i] = new MyThread(i, gc, spq[i%6]);
		qt[i].start();
	}
	for(int i = 0; i < tnum; ++i)
	{
		try {
			qt[i].join();
		}
		catch(InterruptedException e) {
			e.printStackTrace();
		}
	}
		
	//try {
		//Thread.sleep(1000);
	//}
	//catch(InterruptedException e) {
		//e.printStackTrace();
	//}
		// unload this database.
		//gc.unload("dbpedia");
		gc.unload("lubm");
		
	    // also, you can load some exist database directly and then query.

		//sparql = "delete where "
				//+ "{"
				//+ "<http://www.founder/121> ?predicate ?object ."
				//+ "}";				
		//sparql="select ?s where { ?s ?p ?o . }";
		//PERFORMANCE: if we use the query above(as comment), result will be very large and the time cost is large, too
		//The method to improve it is to receive a line and output/save to file at once, instead of combining all lines into a String
		//The related code is in api/http/java/src/jgsc/GstoreConnector.java
	}
}

