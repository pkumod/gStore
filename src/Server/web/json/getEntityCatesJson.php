<?php
/**
 * Created by PhpStorm.
 * User: hanzhe
 * Date: 16-11-22
 * Time: 下午9:51
 */
include 'php_db_com.php';


if (!isset($_GET["id"])) {
    echo 'no';
    return;
}
$eid = $_GET["id"];


//1. 获取top 实体
$sql = "select cateId from baidubaikeEntityCategory where pageId=$eid";
//echo $sql . '<br>';
$cates = json_decode(getResult_json($sql), true);
//echo (jsonFormat($cates, '    ')). '<br>';

//2. 获取实体的头像/热度等信息
$nodes = array();
foreach ($cates as $cate) {
    $cateid = $cate["cateId"];
    $catename = getValue_db("select cateName from cateFreq2 where cateId=$cateid");
    $ispopcate = getValue_db("select COALESCE(cateTagId,0) from baidubaikeCategoryTagList where cateName='$catename'");
//    echo (jsonFormat($result, '    ')) . '<br>';
//    echo $result . '<br>';
    // 将节点信息加入nodejson
    array_push($nodes, array("cateId"=>$cateid, "cateName"=>$catename, "popCateId"=>$ispopcate));
}
// 将hotness 变成百分制

$result = array("nodes"=>$nodes);
echo jsonFormat($result, '    ');
mysqli_close($con);
