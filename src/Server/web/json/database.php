<?php
header("Content-Type: text/html; charset=utf-8");
$mysql_server_name="localhost"; //数据库服务器名称
$mysql_username="root"; // 连接数据库用户名
$mysql_password="icstwip"; // 连接数据库密码
$mysql_database="baidubaikeHTML"; // 数据库的名字
$con=mysqli_connect($mysql_server_name, $mysql_username, $mysql_password, $mysql_database);

mysqli_query($con, "SET NAMES utf8"); // avoid chinese error while inserting into mysql
?>

