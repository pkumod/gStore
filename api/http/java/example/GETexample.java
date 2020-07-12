/*
# Filename: GETexample.java
# Author: suxunbin
# Last Modified: 2019-5-17 10:30
# Description: a simple GET-example of java API
*/

import jgsc.GstoreConnector;

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
// "GET" is a default parameter that can be omitted
public class GETexample {
    public static void main(String[] args) {
        String IP = "127.0.0.1";
        int Port = 9000;
        String username = "root";
        String password = "123456";
        String sparql = "select ?x where"
                        + "{"
                        + "?x  <rdf:type> <ub:UndergraduateStudent>."
                        + "?y    <ub:name> <Course1>."
                        + "?x    <ub:takesCourse>  ?y."
                        + "?z   <ub:teacherOf>    ?y."
                        + "?z    <ub:name> <FullProfessor1>."
                        + "?z    <ub:worksFor>    ?w."
                        + "?w    <ub:name>    <Department0>."
                        + "}";
        String filename = "res.txt";

        // start a gc with given IP, Port, username and password
        GstoreConnector gc = new GstoreConnector(IP, Port, username, password);

        // build a database with a RDF graph
        String res = gc.build("lubm", "data/lubm/lubm.nt");
        System.out.println(res);

        // load the database
        res = gc.load("lubm");
        System.out.println(res);

        // to add, delete a user or modify the privilege of a user, operation must be done by the root user
        //res = gc.user("add_user", "user1", "111111");
        //System.out.println(res);

        // show all users
        res = gc.showUser();
        System.out.println(res);

        // query
        res = gc.query("lubm", "json", sparql);
        System.out.println(res);

        // query and save the result in a file
        gc.fquery("lubm", "json", sparql, filename);

        // save the database if you have changed the database
        res = gc.checkpoint("lubm");
        System.out.println(res);

        // show information of the database
        res = gc.monitor("lubm");
        System.out.println(res);

        // show all databases
        res = gc.show();
        System.out.println(res);

        // export the database
        res = gc.exportDB("lubm", "export/lubm/lubm_get.nt");
        System.out.println(res);

        // unload the database
        res = gc.unload("lubm");
        System.out.println(res);

        // drop the database
        res = gc.drop("lubm", false); //delete the database directly
        //res = gc.drop("lubm", true); //leave a backup
        System.out.println(res);

        // get CoreVersion and APIVersion
        res = gc.getCoreVersion();
        System.out.println(res);
        res = gc.getAPIVersion();
        System.out.println(res);
    }
}
