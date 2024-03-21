<?php
/**
 * 从database 取出候选的实体给用户
 * Created by PhpStorm.
 * User: hanzhe
 * Date: 16-11-29
 * Time: 下午1:28
 */

include "php_db_com.php";

$query = $_GET['query'];
$size = isset($_GET['size']) ? $_GET['size'] : 6;
$nodes = array();
$nodeIds = array();

function appendEntity($sql) {
    global $nodes, $size, $nodeIds;
    if(sizeof($nodes) >= $size) return;
    $cateNames_json = json_decode(getResult_json($sql), true);
    foreach ($cateNames_json as $cn) {
        if(array_key_exists($cn["id"], $nodeIds))
            continue;
        $nodes[] = $cn;
        $nodeIds[$cn['id']] = 0;
        if(sizeof($nodes) > $size) return;
    }
}
// 0. 先去别名表里面取
appendEntity("select b.uniqTitle as title, a.pageId as id from baidubaikeRedirectTitle a left JOIN baidubaikeEntity b on a.pageId=b.id where a.redirect='$query' LIMIT $size");
//1. 先取名字
appendEntity("select title, EntityId as id from baidubaikeEntityInfo where title ='$query'");
//2. 不够则取名字包含的, 速度太慢，取消
//appendEntity("select title, EntityId as id from baidubaikeEntityInfo where hotness>5 and title!='$query' and title LIKE '%{$query}%' ORDER BY hotness desc limit " . ($size - sizeof($nodes)));
echo jsonFormat(array("results"=>$nodes), '    ');
mysqli_close($con);
?>
