<?php
/*
# Filename: GET-example.php
# Author: suxunbin
# Last Modified: 2019-5-16 15:30
# Description: a simple GET-example of php API
 */

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
// "GET" is a default parameter that can be omitted
require "../src/GstoreConnector.php";

$IP = "127.0.0.1";
$Port = 9000;
$username = "root";
$password = "123456";
$sparql = "select ?x where { ?x <rdf:type> <ub:UndergraduateStudent>. ?y <ub:name> <Course1>. ?x <ub:takesCourse> ?y. ?z <ub:teacherOf> ?y. ?z <ub:name> <FullProfessor1>. ?z <ub:worksFor> ?w. ?w <ub:name> <Department0>. }";
$filename = "res.txt";

// start a gc with given IP, Port, username and password
$gc = new GstoreConnector($IP, $Port, $username, $password);

// build a database with a RDF graph
$res = $gc->build("lubm", "data/lubm/lubm.nt");
echo $res . PHP_EOL;

// load the database
$res = $gc->load("lubm");
echo $res . PHP_EOL;

// to add, delete a user or modify the privilege of a user, operation must be done by the root user
//$res = $gc->user("add_user", "user1", "111111");
//echo $res . PHP_EOL;

// show all users
$res = $gc->showUser();
echo $res. PHP_EOL;

// query
$res = $gc->query("lubm", "json", $sparql);
echo $res. PHP_EOL;

// query and save the result in a file
$gc->fquery("lubm", "json", $sparql, $filename);

// save the database if you have changed the database
$res = $gc->checkpoint("lubm");
echo $res. PHP_EOL;

// show information of the database
$res = $gc->monitor("lubm");
echo $res. PHP_EOL;

// show all databases
$res = $gc->show();
echo $res. PHP_EOL;

// export the database
$res = $gc->exportDB("lubm", "export/lubm/lubm_get.nt");
echo $res . PHP_EOL;

// unload the database
$res = $gc->unload("lubm");
echo $res. PHP_EOL;

// drop the database
$res = $gc->drop("lubm", false); #delete the database directly
//$res = $gc->drop("lubm", true); #leave a backup
echo $res. PHP_EOL;

// get CoreVersion and APIVersion
$res = $gc->getCoreVersion();
echo $res. PHP_EOL;
$res = $gc->getAPIVersion();
echo $res. PHP_EOL;
?>




