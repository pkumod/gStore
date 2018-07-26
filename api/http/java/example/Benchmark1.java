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
        String filename = "result/" + num + ".txt";
        String answer = "";
//		String answer = gc.query("root", "123456", "test", sparql);
        gc.query("root", "123456", "test", sparql, filename); 
        
        FileInputStream in = null;
        try {
            in = new FileInputStream(filename);
            int size = in.available();
            byte[] buffer = new byte[size];
            in.read(buffer);
            in.close();
            answer = new String(buffer);
        } catch (IOException e) {
            System.out.println("error occurs: fail to open " + filename);
        }

        // get result count
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

        // compare the result
		if (rnum != Num)
			correctness = false;
	}
}

public class Benchmark1
{
	public static void main(String[] args)
	{
		GstoreConnector gc = new GstoreConnector("172.31.222.78", 3305);
        gc.load("test", "root", "123456");
		String[] spq = new String[6];
        spq[0] = "select ?x where"
                +"{"
                +"?x   <ub:name> <FullProfessor0> ."
                +"}";
        spq[1] = "select distinct ?x where"
                +"{"
                +"?x <rdf:type>  <ub:GraduateStudent>."
                +"?y <rdf:type>  <ub:GraduateStudent>."
                +"?z <rdf:type>  <ub:GraduateStudent>."
                +"?x <ub:memberOf>  ?z."
                +"?z <ub:subOrganizationOf> ?y."
                +"?x <ub:undergaduateDegreeFrom> ?y."
                +"}";
        spq[2] = "select distinct ?x where"
                +"{"
                +"?x   <rdf:type>  <ub:Course>."
                +"?x   <ub:name>   ?y."
                +"}";
        spq[3] = "select ?x where"
                +"{"
                +"?x   <rdf:type>  <ub:UndergraduateStudent>."
                +"?y   <ub:name>  <Course1>."
                +"?x   <ub:takesCourse>  ?y."
                +"?z   <ub:teacherOf>  ?y."
                +"?z   <ub:name>  <FullProfessor1>."
                +"?z   <ub:worksFor>   ?w."
                +"?w   <ub:name>    <Department0>."
                +"}";
        spq[4] = "select distinct ?x where"
                +"{"
                +"?x   <rdf:type>  <ub:UndergraduateStudent>."
                +"?y   <ub:name>   <Course1>."
                +"?x   <ub:takesCourse>  ?y."
                +"?z   <ub:teacherOf>  ?y."
                +"?z   <ub:name>  <FullProfessor1>."
                +"?z   <ub:worksFor>  ?w."
                +"?w   <ub:name>  <Department0>."
                +"}";
        spq[5] = "select distinct ?x where"
                +"{"
                +"?x    <rdf:type>    <ub:UndergraduateStudent>."
                +"}";


		int[] result = {15, 0, 828, 27, 27, 5916};
		int tnum = 6;
		tnum = 3000;
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

		if (MyThread.correctness == true)
		    System.out.println("The results are correct!");
		else
			System.out.println("The results exist errors!");
	}
}
