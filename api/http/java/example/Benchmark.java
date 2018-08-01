/*=============================================================================
# Filename: Benchmark.java
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2017-12-04 11:24
# Description: 
=============================================================================*/

import java.io.*;
import java.util.*;
import jgsc.GstoreConnector;

class MyThread extends Thread {
	public static boolean correctness = true; 
	//public static int cnum = 0;
	private int num = 0;
	private GstoreConnector gc;
	private String sparql;
	private int rnum = 0;
	public MyThread(int _num, GstoreConnector _gc, String _sparql, int _rnum) {
		num = _num;
		gc = _gc;
		sparql = _sparql;
		rnum = _rnum;
	}
	public void run() {
		Thread t = Thread.currentThread();
		//try {
		//	File f = new File("result/"+num+".txt");
		//	DataOutputStream of = new DataOutputStream(new FileOutputStream(f));
		//	of.writeBytes(t.getName()+" "+t.getId()+"\n");
		//	of.writeBytes("Thread: "+num+"\n");
			String answer = gc.query(sparql, "dbpedia", "root", "123456");
		//	of.writeBytes(answer);
			int m = 0;
			for(int i = 0; i<sparql.length(); ++i)
			{
				if(sparql.charAt(i) == '?')
					++m;
				if(sparql.charAt(i) == '{')
					break;
			}
			int n = 0;
			for(int i = 0; i<answer.length(); ++i)
			{
				if(answer.charAt(i) == '{')
					++n;
			}
			int Num = (n-3)/(m+1);
		//	of.writeBytes("The number of the results: "+Num+"\n");
		//	of.writeBytes("Thread "+t.getName()+" ends!");
		//	of.close();
			if (rnum != Num)
				correctness = false;
		//	if (rnum == Num)
		//	{
		//		synchronized (this){
		//			cnum++;
		//		}
		//	}
		//}
		//catch(IOException e) {
                //       e.printStackTrace();
                //}

		
	}
}

public class Benchmark
{
	public static void main(String[] args)
	{
		GstoreConnector gc = new GstoreConnector("172.31.222.94", 9000);
		//gc.load("dbpedia");
		String[] spq = new String[6];
		spq[0] = "select ?v0 where"
			+"{"
			+"?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/class/yago/LanguagesOfBotswana> ."
			+"?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type>    <http://dbpedia.org/class/yago/LanguagesOfNamibia> ."
			+"?v0 <http://www.w3.org/1999/02/22-rdf-syntax-ns#type> <http://dbpedia.org/ontology/Language> ."
			+"}";
		spq[1] = "select ?v0 where"
			+"{"
			+"?v0 <http://dbpedia.org/ontology/associatedBand> <http://dbpedia.org/resource/LCD_Soundsystem> ."
			+"}";
		spq[2] = "select ?v2 where"
			+"{"
			+"<http://dbpedia.org/resource/!!Destroy-Oh-Boy!!> <http://dbpedia.org/property/title> ?v2 ."
			+"}";
		spq[3] = "select ?v0 ?v2 where"
			+"{"
			+"?v0 <http://dbpedia.org/ontology/activeYearsStartYear> ?v2 ."
			+"}";
		spq[4] = "select ?v0 ?v1 ?v2 where"
			+"{"
			+"?v0 <http://dbpedia.org/property/dateOfBirth> ?v2 ."
			+"?v1 <http://dbpedia.org/property/genre> ?v2 ."
			+"}";
		spq[5] = "select ?v0 ?v1 ?v2 ?v3 where"
			+"{"
			+"?v0 <http://dbpedia.org/property/familycolor> ?v1 ."
			+"?v0 <http://dbpedia.org/property/glotto> ?v2 ."
			+"?v0 <http://dbpedia.org/property/lc> ?v3 ."
			+"}";
                //spq[6] = "select ?v0 ?v1 ?v2 ?v3 ?v4 ?v5 ?v6 ?v7 ?v8 ?v9 where"
                //        +"{"
                //        +"?v0 <http://dbpedia.org/property/dateOfBirth> ?v1 ."
                //        +"?v0 <http://dbpedia.org/property/genre> ?v2 ."
                //        +"?v0 <http://dbpedia.org/property/instrument> ?v3 ."
                //        +"?v0 <http://dbpedia.org/property/label> ?v4 ."
                //        +"?v0 <http://dbpedia.org/property/placeOfBirth> ?v5 ."
                //        +"?v6 <http://dbpedia.org/property/name> ?v7 ."
                //        +"?v6 <http://dbpedia.org/property/occupation> ?v8 ."
                //        +"?v6 <http://dbpedia.org/property/placeOfBirth> ?v5 ."
                //        +"?v6 <http://dbpedia.org/property/instrument> ?v3 ."
                //        +"?v6 <http://dbpedia.org/property/notableInstruments> ?v9 ."
                //        +"}";
		int[] result = {10, 14, 14, 199424, 33910, 1039};
		int tnum = 6;
		tnum = 100;
		MyThread[] qt = new MyThread[tnum];
		for(int i = 0; i < tnum; ++i)
		{
			qt[i] = new MyThread(i, gc, spq[i%6], result[i%6]);
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
		//gc.unload("dbpedia");
		//double accuracy = MyThread.cnum / tnum;
		try{
			File f = new File("result/correctness.txt");
			DataOutputStream of = new DataOutputStream(new FileOutputStream(f));
			if (MyThread.correctness == true)
				of.writeBytes("The results are correct!");
			else
				of.writeBytes("The results exist errors!");
			of.close();
		}
		catch(IOException e) {
			e.printStackTrace();
		}         
	}
}
