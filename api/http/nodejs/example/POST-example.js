/*
# Filename: POST-example.js
# Author: suxunbin
# Last Modified: 2019-5-20 17:26
# Description: a simple POST-example of nodejs API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
const GstoreConnector = require("../src/GstoreConnector.js");

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
    "ghttp",
    "root",
    "123456"
);

(async function () {
    // check server
    var res = await gc.check("POST");
    console.log(JSON.stringify(res, ","));

    // login server
    res = await gc.login("POST");
    console.log(JSON.stringify(res, ","));

    // query version
    res = await gc.getCoreVersion("POST");
    console.log(JSON.stringify(res, ","));

    // query version
    res = await gc.stat("POST");
    console.log(JSON.stringify(res, ","));

    // build a database with a RDF graph
    res = await gc.build("lubm", "data/lubm/lubm.nt", "", "", "POST");
    console.log(JSON.stringify(res, ","));

    // load the database
    res = await gc.load("lubm", "0", "POST");
    console.log(JSON.stringify(res, ","));

    // show all users
    res = await gc.showuser("POST");
    console.log(JSON.stringify(res, ","));

    // query
    res = await gc.query("lubm", "json", sparql, "POST");
    console.log(JSON.stringify(res, ","));

    // save the database if you have changed the database
    res = await gc.checkpoint("lubm", "POST");
    console.log(JSON.stringify(res, ","));

    // show information of the database
    res = await gc.monitor("lubm", "POST");
    console.log(JSON.stringify(res, ","));

    // show all databases
    res = await gc.show("POST");
    console.log(JSON.stringify(res, ","));

    // export the database
    res = await gc.exportDB("lubm", "export/lubm/post", "POST");
    console.log(JSON.stringify(res, ","));

    // insert nt
    res = await gc.batchInsert("lubm", "data/bbug/bbug.nt", "", "", "", "POST");
    console.log(JSON.stringify(res, ","));
    res = await gc.checkOperationState(res.opt_id, "POST");
    console.log(JSON.stringify(res, ","));

    // remove nt
    res = await gc.batchRemove("lubm", "data/bbug/bbug.nt", "", "", "POST");
    console.log(JSON.stringify(res, ","));
    res = await gc.checkOperationState(res.opt_id, "POST");
    console.log(JSON.stringify(res, ","));

    // user mange
    res = await gc.showuser("POST");
    console.log(JSON.stringify(res, ","));

    // add usr
    res = await gc.usermanage("1", "test", "123456", "POST");
    console.log(JSON.stringify(res, ","));

    // set user privile
    res = await gc.userprivilegemanage("1", "test", "1,2,3", "lubm", "POST");
    console.log(JSON.stringify(res, ","));

    // clean user privile
    res = await gc.userprivilegemanage("3", "test", "", "", "POST");
    console.log(JSON.stringify(res, ","));

    // update user password
    res = await gc.userpassword("test", "123456", "111111", "POST");
    console.log(JSON.stringify(res, ","));

    // delete user
    res = await gc.usermanage("2", "test", "", "POST");
    console.log(JSON.stringify(res, ","));

    // reason Mange
    var reason_info = "{\"rulename\":\"1\",\"description\":\"互相关注的人为朋友\",\"isenable\": 1,\"type\": 1,\"logic\": 1,\"conditions\": [{\"patterns\": [{\"subject\": \"?x\",	\"predicate\": \"<关注>\",\"object\":\"?y\"},{\"subject\": \"?y\",\"predicate\": \"<关注>\",\"object\": \"?x\"}],\"filters\": [],\"count_info\": {}}],\"return\": {\"source\": \"?x\",\"target\": \"?k\",\"label\": \"朋友\",\"value\": \"\"}}";
    await gc.build("friend_reason", "scripts/test/parser_test/parser_d33.ttl", "", "", "POST");
    await gc.load("friend_reason", "0", "POST");
    res = await gc.show("POST");
    console.log(JSON.stringify(res, ","));

    // add reason
    res = await gc.addReason("1", "friend_reason", reason_info);
    console.log(JSON.stringify(res, ","));

    // list reason
    res = await gc.reasonManage("2", "friend_reason");
    console.log(JSON.stringify(res, ","));

    // compile reason
    res = await gc.reasonManage("3", "friend_reason", "1");
    console.log(JSON.stringify(res, ","));

    // excute reason
    res = await gc.reasonManage("4", "friend_reason", "1");
    console.log(JSON.stringify(res, ","));

    // disable reason
    res = await gc.reasonManage("5", "friend_reason", "1");
    console.log(JSON.stringify(res, ","));

    // show reason
    res = await gc.reasonManage("6", "friend_reason", "1");
    console.log(JSON.stringify(res, ","));

    // delete reason
    res = await gc.reasonManage("7", "friend_reason", "1");
    console.log(JSON.stringify(res, ","));

    await gc.unload("friend_reason");
    await gc.drop("friend_reason", false);

    // unload the database
    res = await gc.unload("lubm", "POST");
    console.log(JSON.stringify(res, ","));

    // drop the database
    res = await gc.drop("lubm", false, "POST"); //delete the database directly
    //res = await gc.drop("lubm", true, "POST") //leave a backup
    console.log(JSON.stringify(res, ","));
})();
