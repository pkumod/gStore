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
		GstoreConnector gc = new GstoreConnector("172.31.222.93", 9016);

		//below are for parallel test
		//GstoreConnector gc = new GstoreConnector("172.31.222.94", 9000);
		//String[] sparql = new String[6];
		//sparql[0] = "select ?v0 where"
			//+"{"
			//+"?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfBotswana> ."
			//+"?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>    <http://dbpedia.org/class/yago/LanguagesOfNamibia> ."
			//+"?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Language> ."
			//+"}";
		//sparql[1] = "select ?v0 where"
			//+"{"
			//+"?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> ."
			//+"}";
		//sparql[2] = "select ?v2 where"
			//+"{"
			//+"<http://dbpedia.org/resource/!!Destroy-Oh-Boy!!> <http://dbpedia.org/property/title> ?v2 ."
			//+"}";
		//sparql[3] = "select ?v0 ?v2 where"
			//+"{"
			//+"?v0 <http://dbpedia.org/ontology/activeYearsStartYear> ?v2 ."
			//+"}";
		//sparql[4] = "select ?v0 ?v1 ?v2 where"
			//+"{"
			//+"?v0 <http://dbpedia.org/property/dateOfBirth> ?v2 ."
			//+"?v1 <http://dbpedia.org/property/genre> ?v2 ."
			//+"}";
		//sparql[5] = "select ?v0 ?v1 ?v2 ?v3 where"
			//+"{"
			//+"?v0 <http://dbpedia.org/property/familycolor> ?v1 ."
			//+"?v0 <http://dbpedia.org/property/glotto> ?v2 ."
			//+"?v0 <http://dbpedia.org/property/lc> ?v3 ."
			//+"}";
		//int[] result = {10, 14, 14, 199424, 33910, 1039};
		//int tnum = 6;
		////tnum = 3000;
		//boolean correctness=true;
		//for(int i = 0; i < tnum; ++i)
		//{
			//String answer = gc.query(sparql[i%6]);
			//int m = 0;
			//for(int ii = 0; ii<sparql[i%6].length(); ++ii)
			//{
				//if(sparql[i%6].charAt(ii) == '?')
					//++m;
				//if(sparql[i%6].charAt(ii) == '{')
					//break;
			//}
			//int n = 0;
			//for(int ii = 0; ii<answer.length(); ++ii)
			//{
				 //if(answer.charAt(ii) == '{')
					//++n;
			//}
			//int Num = (n-3)/(m+1);
			//if(Num!=result[i%6])
				//correctness =false;
			//System.out.println(i+"\n");
		//}
		//if(correctness == true)
			//System.out.println("correct!");

		//for sparql endpoint, URL can also be used here, like freebase.gstore-pku.com:80
		//GstoreConnector gc = new GstoreConnector("tourist.gstore-pku.com", 80);

		//test download function
		//String file = gc.test_download("big.txt");
		//String file = gc.test_download("big.txt");
		//System.out.println(file.substring(file.length()-100, file.length()));
		
	    // build a new database by a RDF file.
	    // note that the relative path is related to gserver.
		gc.build("LUBM10", "data/lubm/LUBM_10.n3", "root", "123456");
	    gc.load("LUBM10", "root", "123456");
		
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
		String answer = gc.query("root", "123456", "LUBM10", sparql);
		System.out.println(answer);
		
		// unload this database.
		gc.unload("LUBM10", "root", "123456");
		
	    // also, you can load some exist database directly and then query.
	    gc.load("LUBM10", "root", "123456");

		//sparql = "delete where "
				//+ "{"
				//+ "<http://www.founder/121> ?predicate ?object ."
				//+ "}";				
		//sparql="select ?s where { ?s ?p ?o . }";
		//PERFORMANCE: if we use the query above(as comment), result will be very large and the time cost is large, too
		//The method to improve it is to receive a line and output/save to file at once, instead of combining all lines into a String
		//The related code is in api/http/java/src/jgsc/GstoreConnector.java
	    answer = gc.query("root", "123456", "LUBM10", sparql);	    
		System.out.println(answer);
			
		//monitor a database
		answer = gc.monitor("LUBM10");
		System.out.println(answer);
		//add a user(with username: Jack, password: 2)
		answer = gc.user("add_user", "root", "123456", "Jack", "2");
		System.out.println(answer);
		//add privilege to user Jack(add_query, add_load, add_unload)
		answer = gc.user("add_query", "root", "123456", "Jack", "LUBM10");
		System.out.println(answer);
		//then Jack can query the database LUBM10
		answer = gc.query("Jack", "2", "LUBM10", sparql);
		System.out.println(answer);
		//delete privilege of user Jack(delete_query, delete_load, delete_unload)
		answer = gc.user("delete_query", "root", "123456", "Jack", "LUBM10");
		System.out.println(answer);
		//delete user(with username: Jack, password: 2)
		answer = gc.user("delete_user", "root", "123456", "Jack", "2");
		System.out.println(answer);

		gc.unload("LUBM10", "root", "123456");
	
	}
}

