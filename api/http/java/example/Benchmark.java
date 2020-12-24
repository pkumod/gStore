/*=============================================================================
# Filename: Benchmark.java
# Author: suxunbin
# Last Modified: 2019-5-17 10:51
# Description: a simple example of multi-thread query
=============================================================================*/

import java.io.*;
import java.util.*;
import jgsc.GstoreConnector;

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)
// default db_name: lubm(must be built in advance)
class MyThread extends Thread {
    public static String IP = "127.0.0.1";
    public static int Port = 9000;
    public static String username = "root";
    public static String password = "123456";
    public static boolean correctness = true;
    private int num = 0;
    private int rnum = 0;
    private String sparql;
    private String filename;
    private String RequestType;

    public MyThread(int _num, int _rnum, String _sparql, String _filename, String _RequestType) {
        num = _num;
        rnum = _rnum;
        sparql = _sparql;
        filename = _filename;
        RequestType = _RequestType;
    }
    public void run() {
        // query
        GstoreConnector gc = new GstoreConnector(IP, Port, username, password);
        String res = gc.query("lubm", "json", sparql, RequestType);

        // fquery
        /*GstoreConnector gc = new GstoreConnector(IP, Port, username, password);
        gc.fquery("lubm", "json", sparql, filename, RequestType);
        String res = "";
        FileInputStream in = null;
        try {
            in = new FileInputStream(filename);
            int size = in.available();
            byte[] buffer = new byte[size];
            in.read(buffer);
            in.close();
            res = new String(buffer);
        } catch (IOException e) {
            System.out.println("error occurs: fail to open " + filename);
        }*/

        // count the num
        int m = 0;
        for (int i = 0; i < sparql.length(); ++i) {
            if (sparql.charAt(i) == '?')
                ++m;
            if (sparql.charAt(i) == '{')
                break;
        }
        int n = 0;
        for (int i = 0; i < res.length(); ++i) {
            if (res.charAt(i) == '{')
                ++n;
        }
        int Num = (n - 3) / (m + 1);

        // compare the result
        if (rnum != Num) {
            correctness = false;
            System.out.println("sparql: " + sparql);
            System.out.println("Num: " + Num);
        }
    }
}

public class Benchmark {
    public static void main(String[] args) {
        int tnum = 1000;
        String RequestType = "POST";

        int[] result = {15, 0, 828, 27, 27, 5916};
        String[] sparql = new String[6];

        sparql[0] = "select ?x where"
                    + "{"
                    + "?x <ub:name> <FullProfessor0> ."
                    + "}";
        sparql[1] = "select distinct ?x where"
                    + "{"
                    + "?x <rdf:type>  <ub:GraduateStudent>."
                    + "?y <rdf:type>  <ub:GraduateStudent>."
                    + "?z <rdf:type>  <ub:GraduateStudent>."
                    + "?x <ub:memberOf>  ?z."
                    + "?z <ub:subOrganizationOf> ?y."
                    + "?x <ub:undergaduateDegreeFrom> ?y."
                    + "}";
        sparql[2] = "select distinct ?x where"
                    + "{"
                    + "?x   <rdf:type>  <ub:Course>."
                    + "?x   <ub:name>   ?y."
                    + "}";
        sparql[3] = "select ?x where"
                    + "{"
                    + "?x   <rdf:type>  <ub:UndergraduateStudent>."
                    + "?y   <ub:name>  <Course1>."
                    + "?x   <ub:takesCourse>  ?y."
                    + "?z   <ub:teacherOf>  ?y."
                    + "?z   <ub:name>  <FullProfessor1>."
                    + "?z   <ub:worksFor>   ?w."
                    + "?w   <ub:name>    <Department0>."
                    + "}";
        sparql[4] = "select distinct ?x where"
                    + "{"
                    + "?x   <rdf:type>  <ub:UndergraduateStudent>."
                    + "?y   <ub:name>   <Course1>."
                    + "?x   <ub:takesCourse>  ?y."
                    + "?z   <ub:teacherOf>  ?y."
                    + "?z   <ub:name>  <FullProfessor1>."
                    + "?z   <ub:worksFor>  ?w."
                    + "?w   <ub:name>  <Department0>."
                    + "}";
        sparql[5] = "select distinct ?x where"
                    + "{"
                    + "?x    <rdf:type>    <ub:UndergraduateStudent>."
                    + "}";

        MyThread[] qt = new MyThread[tnum];
        for (int i = 0; i < tnum; ++i) {
            String filename = "result/res" + i + ".txt";
            qt[i] = new MyThread(i, result[i % 6], sparql[i % 6], filename, RequestType);
            qt[i].start();
        }
        for (int i = 0; i < tnum; ++i) {
            try {
                qt[i].join();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        if (MyThread.correctness == true)
            System.out.println("The answers are correct!");
        else
            System.out.println("The answers exist errors!");
    }
}
