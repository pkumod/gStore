<?php
/*
# Filename: phpAPIExample.php
# Author: yangchaofan
# Last Modified: 2018-7-27 21:50
# Description: a simple example of php API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)

require "../src/GstoreConnector.php";

// example 
$username = "root";
$password = "123456";
$filename = "res.txt";
$sparql = "select ?x where { ?x  <ub:name> <FullProfessor0> .}";

// start a gc
$gc = new GstoreConnector("172.31.222.94", 9000);

// build database
$ret = $gc->build("lubm", "data/lubm/lubm.nt", $username, $password); 
echo $ret . PHP_EOL;

// load rdf
$ret = $gc->load("lubm", $username, $password);
echo $ret . PHP_EOL;

// unload rdf
//$ret = $gc->unload("test", $username, $password);
//echo $ret . PHP_EOL;

// query
echo $gc->query($username, $password, "lubm", $sparql) . PHP_EOL;

// fquery--make a SPARQL query and save the result in the file     
$gc->fquery($username, $password, "lubm", $sparql, $filename);

// show user information
$ret = $gc->showUser();
echo $ret. PHP_EOL;

// show 
$ret = $gc->show($username, $password);
echo $ret. PHP_EOL;

// monitor the database
$ret = $gc->monitor("lubm", $username, $password);
echo $ret. PHP_EOL;

// save the database
$ret = $gc->checkpoint("lubm", $username, $password);
echo $ret. PHP_EOL;

?>
