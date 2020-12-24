<?php
require 'GstoreConnector.php';
$host = '127.0.0.1';
$port = 3305;
$dbname = "LUBM10";
$dbpath = "./data/lubm/lubm.nt";
$query1 = "select ?x where
{
    ?x  <ub:name>   <FullProfessor0>.
}";
$build= new Connector($host,$port);
$build->build($dbname, $dbpath);
$load = new Connector($host,$port);
$load->load($dbname);
$query = new Connector($host,$port);
$result = $query->query($query1);
echo $result;
$load->unload($dbname);
?>

