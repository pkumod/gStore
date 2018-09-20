<?php
/*
# Filename: GstoreConnector.php
# Author: yangchaofan
# Last Modified: 2018-7-18 14:50
# Description: http api for php
*/

class GstoreConnector {
    var $serverIP;
    var $serverPort;
    var $Url;

    function __construct($ip, $port) {
        $this->serverIP = $ip;
        $this->serverPort = $port;
        $this->Url = "http://" . $ip . ":" . strval($port);
    }
    
    function Encode($url) {
        $ret = "";
        for ($i = 0; $i < strlen($url); $i++) {
            $c = $url[$i];
            if (ord($c)==42 or ord($c)==45 or ord($c)==46 or ord($c)==47 or ord($c)==58 or ord($c)==95)
                $ret .= $c;
            else if (ord($c)>=48 and ord($c)<=57)
                $ret .= $c;
            else if (ord($c)>=65 and ord($c)<=90)
                $ret .= $c;
            else if (ord($c)>=97 and ord($c)<=122)
                $ret .= $c;
            else if (ord($c)==32)
                $ret .= "+";
            else 
                $ret .= sprintf("%%%X", ord($c));        
        }
        return $ret;
    }

    function Get($url) {
        $curl = curl_init();
        curl_setopt($curl, CURLOPT_URL,$this->Encode($url));
        curl_setopt($curl, CURLOPT_HEADER, false);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($curl, CURLOPT_TIMEOUT, 10);
        $res = curl_exec($curl);
        if ($res == FALSE) {
            echo "CURL ERROR: ".curl_error($curl).PHP_EOL;
        }
        curl_close($curl);
        return $res;
    }

    function fGet($url, $filename){
        $curl = curl_init();
        curl_setopt($curl, CURLOPT_BUFFERSIZE, 4096);
        curl_setopt($curl, CURLOPT_URL, $this->Encode($url));
        $fp = fopen($filename, "w");
        if (!$fp) {
            echo "open file failed".PHP_EOL;
            return;
        }
        curl_setopt($curl, CURLOPT_HEADER, false);
        curl_setopt($curl, CURLOPT_FILE, $fp);
        curl_exec($curl);
        curl_close($curl);
        fclose($fp);
        return;
    }

    function load($db_name, $username, $password) {
        $cmd = $this->Url . "/?operation=load&db_name=" . $db_name . "&username=" . $username . "&password=" . $password;
        $res = $this->Get($cmd);
        echo $res.PHP_EOL;
        if ($res == "load database done.")
            return true;
        return false;
    }

    function unload($db_name, $username, $password) {
        $cmd = $this->Url . "/?operation=unload&db_name=" . $db_name . "&username=" . $username . "&password=" . $password;
        $res = $this->Get($cmd);
        echo $res.PHP_EOL;
        if ($res == "unload database done.")
            return true;
        return false;
    }

    function build($db_name, $rdf_file_path, $username, $password) {
        $cmd = $this->Url . "/?operation=build&db_name=" . $db_name . "&ds_path=" . $rdf_file_path . "&username=" . $username . "&password=" . $password;
        $res = $this->Get($cmd);
        echo $res.PHP_EOL;
        if ($res == "import RDF file to database done.")
            return true;
        return false;
    }

    function query($username, $password, $db_name, $sparql) {
        $cmd = $this->Url . "/?operation=query&username=" . $username . "&password=" . $password . "&db_name=" . $db_name . "&format=json&sparql=" . $sparql;
        return $this->Get($cmd);
    }

    function fquery($username, $password, $db_name, $sparql, $filename) {
        $cmd = $this->Url . "/?operation=query&username=" . $username . "&password=" . $password . "&db_name=" . $db_name . "&format=json&sparql=" . $sparql;
        $this->fGet($cmd, $filename);
        return;
    }
}
?>
