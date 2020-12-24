/*
# Filename: POST-example.js
# Author: suxunbin
# Last Modified: 2019-5-20 17:26
# Description: a simple POST-example of nodejs API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
const GstoreConnector = require("../GstoreConnector.js");

const sparql = "select ?x where{" +
    "?x  <rdf:type> <ub:UndergraduateStudent>. " +
    "?y    <ub:name> <Course1>. " +
    "?x    <ub:takesCourse>  ?y. " +
    "?z   <ub:teacherOf>    ?y. " +
    "?z    <ub:name> <FullProfessor1>. " +
    "?z    <ub:worksFor>    ?w. " +
    "?w    <ub:name>    <Department0>. }";

// start a gc with given IP, Port, username and password
const gc = new GstoreConnector(
    "127.0.0.1",
    9000,
    "root",
    "123456"
);

(async function() {
    // build a database with a RDF graph
    var res = await gc.build("lubm", "data/lubm/lubm.nt", "POST");
    console.log(JSON.stringify(res,","));

    // load the database
    res = await gc.load("lubm", "POST");
    console.log(JSON.stringify(res,","));

    // to add, delete a user or modify the privilege of a user, operation must be done by the root user
    //res = await gc.user("add_user", "user1", "111111", "POST");
    //console.log(JSON.stringify(res,","));

    // show all users
    res = await gc.showUser("POST");
    console.log(JSON.stringify(res,","));

    // query
    res = await gc.query("lubm", "json", sparql, "POST");
    console.log(JSON.stringify(res,","));

    // save the database if you have changed the database
    res = await gc.checkpoint("lubm", "POST");
    console.log(JSON.stringify(res,","));

    // show information of the database
    res = await gc.monitor("lubm", "POST");
    console.log(JSON.stringify(res,","));

    // show all databases
    res = await gc.show("POST");
    console.log(JSON.stringify(res,","));

    // export the database
    res = await gc.exportDB("lubm", "export/lubm/lubm_post.nt", "POST");
    console.log(JSON.stringify(res,","));

    // unload the database
    res = await gc.unload("lubm", "POST");
    console.log(JSON.stringify(res,","));

    // drop the database
    res = await gc.drop("lubm", false, "POST"); //delete the database directly
    //res = await gc.drop("lubm", true, "POST") //leave a backup
    console.log(JSON.stringify(res,","));

    // get CoreVersion and APIVersion
    res = await gc.getCoreVersion("POST");
    console.log(JSON.stringify(res,","));
    res = await gc.getAPIVersion("POST");
    console.log(JSON.stringify(res,","));
})();
