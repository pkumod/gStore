<?php
/**
 * Created by PhpStorm.
 * User: hanzhe
 * Date: 16-11-22
 * Time: 下午9:52
 */
//ini_set('display_errors', 1);
//ini_set('display_startup_errors', 1);
//error_reporting(E_ALL);

header("Content-Type: text/html; charset=utf-8");
// 指定允许其他域名访问
header('Access-Control-Allow-Origin:*');
// 响应类型
header('Access-Control-Allow-Methods:POST');
// 响应头设置
header('Access-Control-Allow-Headers:x-requested-with,content-type');

include 'database.php';

/** 将数组元素进行urlencode
 * @param String $val
 */
function jsonFormatProtect(&$val){
    if($val!==true && $val!==false && $val!==null){
        $val = urlencode($val);
    }
}
//将json文件排版成标准格式输出
function jsonFormat($data, $indent=null){
    // 对数组中每个元素递归进行urlencode操作，保护中文字符
    array_walk_recursive($data, 'jsonFormatProtect');
    // json encode
    $data = json_encode($data);
    // 将urlencode的内容进行urldecode
    $data = urldecode($data);
    // 缩进处理
    $ret = '';
    $pos = 0;
    $length = strlen($data);
    $indent = isset($indent)? $indent : '    ';
    $newline = "\n";
    $prevchar = '';
    $outofquotes = true;
    for($i=0; $i<=$length; $i++){
        $char = substr($data, $i, 1);
        if($char=='"' && $prevchar!='\\'){
            $outofquotes = !$outofquotes;
        }elseif(($char=='}' || $char==']') && $outofquotes){
            $ret .= $newline;
            $pos --;
            for($j=0; $j<$pos; $j++){
                $ret .= $indent;
            }
        }
        $ret .= $char;
        if(($char==',' || $char=='{' || $char=='[') && $outofquotes){
            $ret .= $newline;
            if($char=='{' || $char=='['){
                $pos ++;
            }
            for($j=0; $j<$pos; $j++){
                $ret .= $indent;
            }
        }
        $prevchar = $char;
    }
    //return JSON.stringify($ret);
    return $ret;
}


function getValue_db($sql){
    global $con;
    $result= mysqli_query($con, $sql);
    return mysqli_fetch_row($result)[0];
}

function getResult_json($sql){
    global $con;
    $sth = mysqli_query($con, $sql);
    $rows = array();
    while($r = mysqli_fetch_assoc($sth)) {
        $rows[] = $r;
    }
    return json_encode($rows);
}

// 将节点信息加入nodejson
function entInfo2Json($entityId, $title) {
    if($entityId <= 0)
        return array("id"=>$entityId, 'name'=>$title, 'type'=>'string','img'=>'', 'description'=>'', 'hotness'=>'');
    $sql = "select IMGUrl, title, FirPara, hotness from baidubaikeEntityInfo where EntityId=$entityId";
    $entInfo = json_decode(getResult_json($sql), true)[0];
    $entityNode = array(
        "id"=>$entityId,
        'name'=>$entInfo["title"],
        'img'=>$entInfo["IMGUrl"],
        'type'=>'entity',
        'description'=>str_replace("\"", "\\\"", str_replace("\\'", "'", $entInfo["FirPara"])),
        'hotness'=>$entInfo["hotness"]);
    return $entityNode;
}

// 将hotness 变成百分制
function normalizeHotness(&$nodejson) {
    $maxHotness = 1;
    foreach ($nodejson as $key => $field) {
        if ($field['hotness']) {
            $maxHotness = max($maxHotness, $nodejson[$key]['hotness']);
        }
    }
    foreach ($nodejson as $key => $field) {
        if ($field['hotness']) {
            $hot100 = $nodejson[$key]['hotness'] / $maxHotness * 100;
            $nodejson[$key]['hotness'] = sqrt(sqrt($hot100 * 100)*100);
        }
    }
}
