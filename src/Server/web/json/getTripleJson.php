<?php
include 'php_db_com.php';

$sid = intval($_GET["id"]);
//如果是按照title输入的，优先按title找
if(isset($_GET["title"])) {
    $sid = getValue_db("select COALESCE(pageId,0) from baidubaikeRedirectTitle where redirect='" . $_GET["title"] . "'");
}

// 输入是id=-1，则随便取一个id
if($sid < 0) {
    $rdId = getValue_db("select FLOOR(RAND()*31000000+45976264");
    $sid = getValue_db("select COLLATE (subjectId, -1) from semiTriple1 where id=$rdId");
}

// title 不存在或者没有对应三元组，则返回false
if($sid == 0 || getValue_db("select COALESCE('a', 'b') from semiTriple1 where subjectId=$sid or objectId=$sid limit 1") != 'a'){
    echo 'no:' . $sid;
    return;
}

// 1.0 获取中心点的节点信息
$nodes=array();
array_push($nodes, entInfo2Json($sid, ''));
$edges = array();
$id2wikiId = array(0=>$sid);
$wikiId2id = array($sid=>0);
$edge2color = array();
$maxObjEntity = 20;
$curObjId = -1; // 客体非实体（普通字符串， id从-1,-2,-3,...开始编号）

function insertAndGetSeqId($wikiId, $title, &$wikiId2id, &$id2wikiId, &$nodes) {
    // object <= 0 : 则 wikiId从-1，,2, ... 开始编号
    global $curObjId;
    if($wikiId <= 0) $wikiId = $curObjId --;
    if(!array_key_exists($wikiId, $wikiId2id)) {
        $subId = sizeof($id2wikiId);
        $id2wikiId[$subId] = $wikiId;
        $wikiId2id[$wikiId] = $subId;
        array_push($nodes, entInfo2Json($wikiId, $title));
    }
    return $wikiId2id[$wikiId];
}

function addNodeAndEdge($sql) {
    global $nodes, $edges, $id2wikiId, $wikiId2id, $edge2color, $con, $sid;
    $result = mysqli_query($con, $sql);
    if(!$result) return;
    while($row=mysqli_fetch_array($result)){
        $pred = $row[1];
        $objWikiId = intval($row[2]);
        $obj = $row[3];
        $subWikiId = intval($row[4]);
        $subj = $row[0];

        if(trim($obj) == '' || $subWikiId == $objWikiId) continue;

        /* 1.0.1 主体点 */
        $subId = insertAndGetSeqId($subWikiId, $subj, $wikiId2id, $id2wikiId, $nodes);
        /* 1.0.2 客体点 */
        $objId = insertAndGetSeqId($objWikiId, $obj, $wikiId2id, $id2wikiId, $nodes);
        /* 1.1 表示边*/
        if(!array_key_exists($pred, $edge2color))
            $edge2color[$pred] = sizeof($edge2color) + 1;
        $color = $edge2color[$pred];
        $edge = array("source"=>$subId, "target"=>$objId, 'type'=>1, 'description'=>$pred, 'color'=>$color);
        array_push($edges, $edge);
    }
    normalizeHotness($nodes);

    /* 2. 查询所有客体之间的关系 */
    $objs = array_keys($wikiId2id);
    $objs = array_diff($objs, array($sid));
    $sql = "select subjectId, predicate, objectId from semiTriple1 where subjectId in (" . implode(',', $objs) . ') and objectId in ('.  implode(',', $objs) . ')';
    $result = mysqli_query($con, $sql);
    if(!$result) return;
    while($row = mysqli_fetch_array($result)){
        $obj1WikiId = $row[0];
        $pred = $row[1];
        $obj2WikiId = $row[2];
        if(!array_key_exists($pred, $edge2color))
            $edge2color[$pred] = sizeof($edge2color) + 1;
        $color = $edge2color[$pred];
        $edge = array("source"=>$wikiId2id[$obj1WikiId], "target"=>$wikiId2id[$obj2WikiId], 'type'=>2, 'description'=>$pred , 'color'=>$color);
        array_push($edges, $edge);
    }
}

$name = getValue_db("select title from baidubaikeEntityInfo where EntityId=$sid");
addNodeAndEdge("select '$name', predicate, objectId, object,subjectId from semiTriple1 where subjectId = " . $sid);
addNodeAndEdge("select '$name', predicate, objectId, object,subjectId from semiTriple1 where objectId = " . $sid . " limit $maxObjEntity");

/* 3. 生成 json */
$re = array("nodes"=>$nodes, "edges"=>$edges);
echo (jsonFormat($re, '    '));
mysqli_close($con);
?>

