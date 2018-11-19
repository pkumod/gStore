<?php
/**
 * 从database 取出候选的实体给用户
 * Created by PhpStorm.
 * User: hanzhe
 * Date: 16-11-29
 * Time: 下午1:28
 */

include "php_db_com.php";

$query = $_GET['query'].'%';
$size = isset($_GET['size']) ? $_GET['size'] : 4;

$nodes = array();
$nodeIds = array();
//1. 先取名字开头的
$sql = "select cateName,cateTagId from baidubaikeCategoryTagList where cateName like '$query%' and pop>10 order by pop desc limit $size";
$cateNames_json = json_decode(getResult_json($sql), true);
foreach ($cateNames_json as $cn) {
    if(array_key_exists($cn["cateTagId"], $nodeIds))
        continue;
    $nodeIds[$cn['cateTagId']] = 0;
    $nodes[] = $cn;
}
if(sizeof($nodes) >= $size) {
    echo jsonFormat(array("results"=>$nodes), '    ');
    return;
}

//2. 不够则取名字包含的
$size = $size - sizeof($nodes);
$sql = "select cateName,cateTagId from baidubaikeCategoryTagList where cateName LIKE '%{$query}%' and pop>10 ORDER by pop desc limit $size";
$cateNames_json = json_decode(getResult_json($sql), true);
foreach ($cateNames_json as $cn) {
    if(array_key_exists($cn["cateTagId"], $nodeIds))
        continue;
    $nodeIds[$cn['cateTagId']] = 0;
    $nodes[] = $cn;
}
echo jsonFormat(array("results"=>$nodes), '    ');
mysqli_close($con);
?>
