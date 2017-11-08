/*
 * JavaAPIExample.java
 *
 *  Created on: 2014-11-4
 *      Author: zengli
 */

import jgsc.GstoreConnector;

// before run this example, you must start up the GStore server at first (use command ./gserver).
public class DBpedia
{
	public static void main(String[] args)
	{
		// initialize the GStore server's IP address and port.
		GstoreConnector gc = new GstoreConnector("dbpedia.gstore-pku.com", 80);
		//for sparql endpoint, URL can also be used here, like freebase.gstore-pku.com:80
		//GstoreConnector gc = new GstoreConnector("tourist.gstore-pku.com", 80);

		//test download function
		//String file = gc.test_download("big.txt");
		//String file = gc.test_download("big.txt");
		//System.out.println(file.substring(file.length()-100, file.length()));
		
	    // build a new database by a RDF file.
	    // note that the relative path is related to gserver.
		//gc.build("LUBM10", "data/LUBM_10.n3");
		//gc.load("LUBM10");
		
		// then you can execute SPARQL query on this database.
		String sparql = "SELECT ?v0 WHERE { ?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> .  }";
		String answer = gc.query(sparql);
		System.out.println(answer);
		
		// unload this database.
		//gc.unload("LUBM10");
	}
}

