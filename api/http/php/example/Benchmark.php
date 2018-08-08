<?php
/*
# Filename: Benchmark.php
# Author: yangchaofan
# Last Modified: 2018-7-28 15:37
# Description: a multi-thread example for php API
*/

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)
require '../src/GstoreConnector.php';

// variable definition
$tnum = 3000;
$correctness = true;

$threads = array();
$result = array(15, 0, 828, 27, 27, 5916);
$sparql = array();
$sparql0 = "select ?x where
           {
           ?x   <ub:name> <FullProfessor0> .
           }";
$sparql1 = "select distinct ?x where
           { 
           ?x <rdf:type>  <ub:GraduateStudent>.
           ?y <rdf:type>  <ub:GraduateStudent>.
           ?y <rdf:type>  <ub:GraduateStudent>.
           ?x <ub:memberOf>  ?z.
           ?z <ub:subOrganizationOf> ?y.
           ?x <ub:undergaduateDegreeFrom> ?y.
           }";
$sparql2 = "select distinct ?x where
           {
           ?x   <rdf:type>  <ub:Course>.
           ?x   <ub:name>   ?y.
           }";
$sparql3 = "select ?x where
           {
           ?x   <rdf:type>  <ub:UndergraduateStudent>.
           ?y   <ub:name>  <Course1>.
           ?x   <ub:takesCourse>  ?y.
           ?z   <ub:teacherOf>  ?y.
           ?z   <ub:name>  <FullProfessor1>.
           ?z   <ub:worksFor>   ?w.
           ?w   <ub:name>    <Department0>.
           }";
$sparql4 = "select distinct ?x where
           {
           ?x   <rdf:type>  <ub:UndergraduateStudent>.
           ?y   <ub:name>   <Course1>.
           ?x   <ub:takesCourse>  ?y.
           ?z   <ub:teacherOf>  ?y.
           ?z   <ub:name>  <FullProfessor1>.
           ?z   <ub:worksFor>  ?w.
           ?w   <ub:name>  <Department0>.
           }";
$sparql5 = "select distinct ?x where
           {
           ?x    <rdf:type>    <ub:UndergraduateStudent>.
           }";



class Mythread extends Thread {
    var $rnum, $sparql, $filename;
    public function __construct($rnum, $sparql, $filename) {
        $this->rnum = $rnum;
        $this->sparql = $sparql;
        $this->filename = $filename;
    }
    public function run () {
        
        // query
        $gc = new GstoreConnector("172.31.222.94", 9000);

        //$gc->build("test", "data/lubm/lubm.nt", "root", "123456");
        //$gc.load("test", "root", "123456");
        $gc->fquery("root", "123456", "lubm", $this->sparql, $this->filename);
        //$res = $gc->query("root", "123456", "test", $sparql);
        
        // read the file to a str
        $f = fopen($this->filename, "r") or exit("fail to open " . $this->filename);
        $res = "";
        while(!feof($f)) {
            $res .= (fgets($f). PHP_EOL);
        }
        fclose($f);

        // count the num
        $m = 0;
        for ($i = 0; $i < strlen($this->sparql); $i++) {
            if ($this->sparql[$i] == "?")
                $m = $m + 1;
            if ($this->sparql[$i] == "{")
                break;
        }   
        $n = 0;
        for ($i = 0; $i < strlen($res); $i++) {
            if ($res[$i] == "{")
                $n = $n + 1;
        }
        $Num = ($n-3)/($m+1);

        // compare the result
        if ($this->rnum != $Num) {
            $correctness = false;
            echo "sparql: " . $this->sparql . PHP_EOL;
            echo "Num: " . strval($Num) . PHP_EOL;
        }
    }
}

# create sparql
array_push($sparql, $sparql0, $sparql1, $sparql2);
array_push($sparql, $sparql3, $sparql4, $sparql5);

# create the threads
for ($i = 0; $i < $tnum; $i++) {
    $filename = "result/" . strval($i) . ".txt";
    $threads[] = new Mythread($result[$i%6], $sparql[$i%6], $filename);
}

foreach ($threads as $i) 
    $i->start();

foreach ($threads as $i)
    $i->join();

if ($correctness == true)
    echo "The answers are correct!" . PHP_EOL;
else
    echo "The answers exist errors!" . PHP_EOL;

echo "Main thread exit" . PHP_EOL;



?>
