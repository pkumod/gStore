<?php
/*
# Filename: GstoreConnector.php
# Author: yangchaofan suxunbin
# Last Modified: 2019-5-16 11:00
# Description: http api for php
 */

class GstoreConnector {
    var $serverIP;
    var $serverPort;
    var $Url;
    var $username;
    var $password;

    function __construct($ip, $port, $user, $passwd) {
        if ($ip == "localhost")
            $this->serverIP = "127.0.0.1";
        else
            $this->serverIP = $ip;
        $this->serverPort = $port;
        $this->Url = "http://" . $ip . ":" . strval($port);
        $this->username = $user;
        $this->password = $passwd;
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
            else if (ord($c)!=9 and ord($c)!=10 and ord($c)!=13)
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

    function Post($url, $strPost) {
        $curl = curl_init();
        curl_setopt($curl, CURLOPT_URL,$this->Encode($url));
        curl_setopt($curl, CURLOPT_HEADER, false);
        curl_setopt($curl, CURLOPT_RETURNTRANSFER, 1);
        curl_setopt($curl, CURLOPT_TIMEOUT, 10);
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_POSTFIELDS, $strPost);
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

    function fPost($url, $strPost, $filename){
        $curl = curl_init();
        curl_setopt($curl, CURLOPT_BUFFERSIZE, 4096);
        curl_setopt($curl, CURLOPT_URL, $this->Encode($url));
        curl_setopt($curl, CURLOPT_POST, true);
        curl_setopt($curl, CURLOPT_POSTFIELDS, $strPost);
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

    function build($db_name, $rdf_file_path, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=build&db_name=" . $db_name . "&ds_path=" . $rdf_file_path . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/build";
            $strPost = "{\"db_name\": \"" . $db_name . "\", \"ds_path\": \"" . $rdf_file_path . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function load($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=load&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/load";
            $strPost = "{\"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function unload($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=unload&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/unload";
            $strPost = "{\"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function user($type, $username2, $addition, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=user&type=" . $type . "&username1=" . $this->username . "&password1=" . $this->password . "&username2=" . $username2 . "&addition=" . $addition;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/user";
            $strPost = "{\"type\": \"" . $type . "\", \"username1\": \"" . $this->username . "\", \"password1\": \"" . $this->password . "\", \"username2\": \"" . $username2 . "\", \"addition\": \"" . $addition . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function showUser($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=showUser&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/showUser";
            $strPost = "{\"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function query($db_name, $format, $sparql, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=query&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&format=" . $format . "&sparql=" . $sparql;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/query";
            $strPost = "{\"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"format\": \"" . $format . "\", \"sparql\": \"" . $sparql . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function fquery($db_name, $format, $sparql, $filename, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=query&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&format=" . $format . "&sparql=" . $sparql;
            $this->fGet($strUrl, $filename);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/query";
            $strPost = "{\"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"format\": \"" . $format . "\", \"sparql\": \"" . $sparql . "\"}";
            $this->fPost($strUrl, $strPost, $filename);
        }
        return;
    }

    function drop($db_name, $is_backup, $request_type="GET") {
        if ($request_type == "GET")
        {
            if ($is_backup)
                $strUrl = $this->Url . "/?operation=drop&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password . "&is_backup=true";
            else
                $strUrl = $this->Url . "/?operation=drop&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password . "&is_backup=false";
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/drop";
            if ($is_backup)
                $strPost = "{\"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"is_backup\": \"true\"}";
            else
                $strPost = "{\"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"is_backup\": \"false\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function monitor($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=monitor&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/monitor";
            $strPost = "{\"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function checkpoint($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=checkpoint&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/checkpoint";
            $strPost = "{\"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function show($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=show&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/show";
            $strPost = "{\"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function getCoreVersion($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=getCoreVersion&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/getCoreVersion";
            $strPost = "{\"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function getAPIVersion($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=getAPIVersion&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/getAPIVersion";
            $strPost = "{\"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }

    function exportDB($db_name, $dir_path, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "/?operation=export&db_name=" . $db_name . "&ds_path=" . $dir_path . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strUrl = $this->Url . "/export";
            $strPost = "{\"db_name\": \"" . $db_name . "\", \"ds_path\": \"" . $dir_path . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($strUrl, $strPost);
        }
        return $res;
    }
}
?>
