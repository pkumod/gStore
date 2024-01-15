<?php
/**
 * Created by PhpStorm.
 * User: hanzhe
 * Date: 16-11-22
 * Time: 下午9:51
 */
include 'php_db_com.php';

$restrict = '';
if(!isset($_GET["title"])) {
    if (!isset($_GET["id"])) {
        echo 'no cate name or id inputed!';
        return;
    }
    else $restrict = 'cateTagId=' . $_GET["id"];
}
else $restrict = 'cateName="' . $_GET["title"] . '"';

// 0. 获得category tag id
$sql = "select COALESCE(cateTagId,-1) from baidubaikeCategoryTagList where " . $restrict . " order by pop desc";
//echo $sql . '<br>';
$cateTagId = getValue_db($sql);
if($cateTagId < 0){
    echo 'cate name not found!';
    return;
}

//1. 获取top 实体
$sql = "select entityId from baidubaikeCategoryPopInstance where cateTagId=$cateTagId limit 20";
//echo $sql . '<br>';
$entities_json = json_decode(getResult_json($sql), true);

//2. 获取实体的头像/热度等信息
$nodes = array();
$maxHotness = 0;
foreach ($entities_json as $ent) {
    $entId = $ent["entityId"];
    // 将节点信息加入nodejson
    array_push($nodes, entInfo2Json($entId, ''));
}
// 将hotness 变成百分制
normalizeHotness($nodes);

$result = array("nodes"=>$nodes);
echo jsonFormat($result, '    ');
mysqli_close($con);
