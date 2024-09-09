<?php
/*
# Filename: POST-example.php
# Author: suxunbin
# Last Modified: 2019-5-16 15:30
# Description: a simple POST-example of php API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
require "../src/GstoreConnector.php";

$IP = "127.0.0.1";
$Port = 9000;
$httpTye = "ghttp";
$username = "root";
$password = "123456";
$sparql = "select ?x where { ?x <rdf:type> <ub:UndergraduateStudent>. ?y <ub:name> <Course1>. ?x <ub:takesCourse> ?y. ?z <ub:teacherOf> ?y. ?z <ub:name> <FullProfessor1>. ?z <ub:worksFor> ?w. ?w <ub:name> <Department0>. }";
$filename = "res.txt";

// start a gc with given IP, Port, username and password
$gc = new GstoreConnector($IP, $Port, $httpTye, $username, $password);

// check
$res = $gc->check("POST");
echo $res . PHP_EOL;

// login server
$res = $gc->login("POST");
echo $res . PHP_EOL;

// query version
$res = $gc->getCoreVersion("POST");
echo $res . PHP_EOL;

$res = $gc->stat("POST");
echo $res . PHP_EOL;

// build a database with a RDF graph
$res = $gc->build("lubm", "data/lubm/lubm.nt", "", "", "POST");
echo $res . PHP_EOL;

// load the database 
$res = $gc->load("lubm", "POST");
echo $res . PHP_EOL;

// show all users
$res = $gc->showuser("POST");
echo $res. PHP_EOL;

// query
$res = $gc->query("lubm", "json", $sparql, "POST");
echo $res. PHP_EOL;

// query and save the result in a file
$gc->fquery("lubm", "json", $sparql, $filename, "POST");

// save the database if you have changed the database
$res = $gc->checkpoint("lubm", "POST");
echo $res. PHP_EOL;

// show information of the database
$res = $gc->monitor("lubm", "POST");
echo $res. PHP_EOL;

// show all databases
$res = $gc->show("POST");
echo $res. PHP_EOL;

// export the database
$res = $gc->exportDB("lubm", "export/lubm/post", "POST");
echo $res . PHP_EOL;

// insert nt
$res = $gc->batchInsert("lubm", "data/bbug/bbug.nt", "", "", "", "POST");
echo $res . PHP_EOL;

// remove nt
$res = $gc->batchRemove("lubm", "data/bbug/bbug.nt", "", "", "POST");
echo $res . PHP_EOL;

// show user
$res = $gc->showuser("POST");
echo $res . PHP_EOL;

// add usr
$res = $gc->usermanage("1", "test", "123456", "POST");
echo $res . PHP_EOL;

// set user privile
$res = $gc->userprivilegemanage("1", "test", "1,2,3", "lubm", "POST");
echo $res . PHP_EOL;

// clean user privile
$res = $gc->userprivilegemanage("3", "test", "", "", "POST");
echo $res . PHP_EOL;

// update user password
$res = $gc->userpassword("test", "123456", "111111", "POST");
echo $res . PHP_EOL;

// delete user
$res = $gc->usermanage("2", "test", "POST");
echo $res . PHP_EOL;

// reason Mange
$reason_info = "{\"rulename\":\"1\",\"description\":\"互相关注的人为朋友\",\"isenable\": 1,\"type\": 1,\"logic\": 1,\"conditions\": [{\"patterns\": [{\"subject\": \"?x \",	\"predicate\": \"<关注>\",\"object\":\"?y\"},{\"subject\": \"?y \",\"predicate\": \"<关注>\",\"object\": \"?x\"}],\"filters\": [],\"count_info\": {}}],\"return\": {\"source\": \"?x\",\"target\": \"?k\",\"label\": \"朋友\",\"value\": \"\"}}";
$res = $gc->build("friend_reason", "scripts/test/parser_test/parser_d33.ttl", "", "", "POST");
$res = $gc->load("friend_reason", "POST");
$res = $gc->show("POST");
echo $res . PHP_EOL;

// add reason
$res = $gc->addReason("1", "friend_reason", reason_info);
echo $res . PHP_EOL;
// list reason
$res = $gc->reasonManage("2", "friend_reason");
echo $res . PHP_EOL;
// compile reason
$res = $gc->reasonManage("3", "friend_reason", "1"));
echo $res . PHP_EOL;
// excute reason
$res = $gc->reasonManage("4", "friend_reason", "1");
echo $res . PHP_EOL;
// disable reason
$res = $gc->reasonManage("5", "friend_reason", "1");
echo $res . PHP_EOL;
// show reason
$res = $gc->reasonManage("6", "friend_reason", "1");
echo $res . PHP_EOL;
// delete reason
$res = $gc->reasonManage("7", "friend_reason", "1");
echo $res . PHP_EOL;
$gc->unload("friend_reason");
$gc->drop("friend_reason", false);

// unload the database
$res = $gc->unload("lubm", "POST");
echo $res. PHP_EOL;

// drop the database
$res = $gc->drop("lubm", false, "POST"); #delete the database directly
//$res = $gc->drop("lubm", true, "POST"); #leave a backup
echo $res. PHP_EOL;

?>




