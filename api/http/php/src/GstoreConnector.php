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

    function __construct($ip, $port, $httpType, $user, $passwd) {
        if ($ip == "localhost")
            $this->serverIP = "127.0.0.1";
        else
            $this->serverIP = $ip;
        $this->serverPort = $port;
        $this->Url = "http://" . $ip . ":" . strval($port) . "/";
        if ($httpType == "grpc")
            $this->Url .= "grpc/api/"; 
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

    function check($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=check";
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"check\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function login($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=login&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"login\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function testConnect($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=testConnect&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"testConnect\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function getCoreVersion($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=getCoreVersion&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"getCoreVersion\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function ipmanage($type, $ip_type, $ips, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=ipmanage&type=" . $type . "&ip_type=" . $ip_type . "&ips=" . $ips . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"ipmanage\", \"type\": \"" . $type . "\", \"ip_type\": \"" . $ip_type . "\", \"ips\": \"" . $ips . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function upload($file_path, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=upload&filepath=" . $file_path . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"upload\", \"filepath\": \"" . $file_path . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function download($file_path, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=download&filepath=" . $file_path . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"download\", \"filepath\": \"" . $file_path . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function stat($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=stat&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"stat\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function shutdown($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=shutdown&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"shutdown\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function show($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=show&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"show\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function load($db_name, $csr, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=load&db_name=" . $db_name . "&csr=" . $csr . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"load\", \"db_name\": \"" . $db_name . "\", \"csr\": \"" . $csr . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function unload($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=unload&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"unload\", \"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function monitor($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=monitor&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"monitor\", \"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function build($db_name, $db_path, $async="", $callback="", $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=build&db_name=" . $db_name . "&db_path=" . $db_path . "&username=" . $this->username . "&password=" . $this->password . "&async=" . $async . "&callback=" . $callback;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"build\", \"db_name\": \"" . $db_name . "\", \"db_path\": \"" . $db_path . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"async\": \"" . $async . "\", \"callback\": \"" . $callback . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }
	
    function drop($db_name, $is_backup, $request_type="GET") {
        if ($request_type == "GET")
        {
            if ($is_backup)
                $strUrl = $this->Url . "?operation=drop&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password . "&is_backup=true";
            else
                $strUrl = $this->Url . "?operation=drop&db_name=" . $db_name . "&username=" . $this->username . "&password=" . $this->password . "&is_backup=false";           
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            if ($is_backup)
                $strPost = "{\"operation\": \"drop\", \"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"is_backup\": \"true\"}";
            else
                $strPost = "{\"operation\": \"drop\", \"db_name\": \"" . $db_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"is_backup\": \"false\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function backup($db_name, $backup_path, $async="", $callback="", $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=backup&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&backup_path=" . $backup_path . "&async=" . $async . "&callback=" . $callback;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"backup\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"backup_path\": \"" . $backup_path . "\", \"async\": \"" . $async . "\", \"callback\": \"" . $callback . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;  
    }

    function backuppath($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=backuppath&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"backuppath\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function restore($db_name, $backup_path, $async="", $callback="", $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=restore&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&backup_path=" . $backup_path . "&async=" . $async . "&callback=" . $callback;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"restore\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"backup_path\": \"" . $backup_path . "\", \"async\": \"" . $async . "\", \"callback\": \"" . $callback . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function query($db_name, $format, $sparql, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=query&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&format=" . $format . "&sparql=" . $sparql; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"query\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"format\": \"" . $format . "\", \"sparql\": \"" . $sparql . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function exportDB($db_name, $db_path, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=export&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&db_path=" . $db_path;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"export\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"db_path\": \"" . $db_path . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function batchInsert($db_name, $file, $dir="", $async="", $callback="", $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=batchInsert&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&file=" . $file . "&dir=" . $dir . "&async=" . $async . "&callback=" . $callback;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"batchInsert\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"file\": \"" . $file . "\", \"dir\": \"" . $dir . "\", \"async\": \"" . $async . "\", \"callback\": \"" . $callback . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function batchRemove($db_name, $file, $async="", $callback="", $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=batchRemove&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&file=" . $file . "&async=" . $async . "&callback=" . $callback;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"batchRemove\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"file\": \"" . $file . "\", \"async\": \"" . $async . "\", \"callback\": \"" . $callback . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function rename($db_name, $new_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=rename&db_name=" . $db_name . "&new_name=" . $new_name . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"rename\", \"db_name\": \"" . $db_name . "\", \"new_name\": \"" . $new_name . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function checkOperationState($opt_id, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=checkOperationState&opt_id=" . $opt_id . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"checkOperationState\", \"opt_id\": \"" . $opt_id . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function begin($db_name, $isolevel, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=begin&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&isolevel=" . $isolevel;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"begin\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"isolevel\": \"" . $isolevel . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function tquery($db_name, $tid, $sparql, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=tquery&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&tid=" . $tid . "&sparql=" . $sparql; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"tquery\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"tid\": \"" . $tid . "\", \"sparql\": \"" . $sparql . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }
    
    function commit($db_name, $tid, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=commit&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&tid=" . $tid;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"commit\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"tid\": \"" . $tid . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function rollback($db_name, $tid, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=rollback&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&tid=" . $tid;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"rollback\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"tid\": \"" . $tid . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;    
    }

    function checkpoint($db_name, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=checkpoint&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"checkpoint\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function showuser($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=showuser&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"showuser\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function usermanage($type, $op_username, $op_password, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=usermanage&type=" . $type . "&username=" . $this->username . "&password=" . $this->password . "&op_username=" . $op_username . "&op_password=" . $op_password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"usermanage\", \"type\": \"" . $type . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"op_username\": \"" . $op_username . "\", \"op_password\": \"" . $op_password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
     }
	
    function userprivilegemanage($type, $op_username, $privileges="", $db_name="", $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=userprivilegemanage&type=" . $type . "&username=" . $this->username . "&password=" . $this->password . "&op_username=" . $op_username . "&privileges=" . $privileges . "&db_name=" . $db_name; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"userprivilegemanage\", \"type\": \"" . $type . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"op_username\": \"" . $op_username . "\", \"privileges\": \"" . $privileges . "\", \"db_name\": \"" . $db_name . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
     }

    function userpassword($re_username, $re_password, $op_password, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=userpassword&username=" . $re_username . "&password=" . re_password . "&op_password=" . $op_password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"userpassword\", \"username\": \"" . $re_username . "\", \"password\": \"" . re_password . "\", \"op_password\": \"" . $op_password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
     }

    function funquery($funInfo, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=funquery&funInfo=" . $funInfo . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"funquery\", \"funInfo\": \"" . $funInfo . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function funcudb($type, $funInfo, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=funcudb&type=" . $type . "&funInfo=" . funInfo . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"funcudb\", \"type\": \"" . $type . "\", \"funInfo\": \"" . funInfo . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function funreview($funInfo, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=funreview&funInfo=" . $funInfo . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"funreview\", \"funInfo\": \"" . $funInfo . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function txnlog($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=txnlog&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"txnlog\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function querylogdate($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=querylogdate&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"querylogdate\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function querylog($date, $pageNo, $pageSize, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=querylog&date=" . $date . "&pageNo=" . $pageNo . "&pageSize=" . $pageSize . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"querylog\", \"date\": \"" . $date . "\", \"pageNo\": \"" . $pageNo . "\", \"pageSize\": \"" . $pageSize . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function accesslogdate($request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=accesslogdate&username=" . $this->username . "&password=" . $this->password; 
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"accesslogdate\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function accesslog($date, $pageNo, $pageSize, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=accesslog&date=" . $date . "&pageNo=" . $pageNo . "&pageSize=" . $pageSize . "&username=" . $this->username . "&password=" . $this->password;
            $res = $this->Get($strUrl);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"accesslog\", \"date\": \"" . $date . "\", \"pageNo\": \"" . $pageNo . "\", \"pageSize\": \"" . $pageSize . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
            $res = $this->Post($this->Url, $strPost);
        }
        return $res;
    }

    function addReason($type, $db_name, $ruleinfo) {
        $strPost = "{\"operation\": \"reasonManage\", \"type\": \"" . $type . "\", \"db_name\": \"" . $db_name . "\", \"ruleinfo\": " . $ruleinfo . ", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
        $res = $this->Post($this->Url, $strPost);
        return $res;
    }

    function reasonManage($type, $db_name, $rulename="") {
        $strPost = "{\"operation\": \"reasonManage\", \"type\": \"" . $type . "\", \"db_name\": \"" . $db_name . "\", \"rulename\": \"" . $rulename . "\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\"}";
        $res = $this->Post($this->Url, $strPost);
        return $res;
    }

    function fquery($db_name, $format, $sparql, $filename, $request_type="GET") {
        if ($request_type == "GET")
        {
            $strUrl = $this->Url . "?operation=query&username=" . $this->username . "&password=" . $this->password . "&db_name=" . $db_name . "&format=" . $format . "&sparql=" . $sparql; 
            $this->fGet($strUrl, $filename);
        }
        elseif ($request_type == "POST")
        {
            $strPost = "{\"operation\": \"query\", \"username\": \"" . $this->username . "\", \"password\": \"" . $this->password . "\", \"db_name\": \"" . $db_name . "\", \"format\": \"" . $format . "\", \"sparql\": \"" . $sparql . "\"}";
            $this->fPost($this->Url, $strPost, $filename);
        }
        return;     
    }
}
?>
