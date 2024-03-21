<?php
ini_set('display_errors', 1);
ini_set('display_startup_errors', 1);
error_reporting(E_ALL);
//header("Content-Type: text/html; charset=utf-8");
// 指定允许其他域名访问
//header('Access-Control-Allow-Origin:*');
// 响应类型
//header('Access-Control-Allow-Methods:POST');
// 响应头设置
//header('Access-Control-Allow-Headers:x-requested-with,content-type');
// open the file in a binary mode
//header("Access-Control-Allow-Origin: *");
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Headers: X-Requested-With, content-type, access-control-allow-origin, access-control-allow-methods, access-control-allow-headers');
$image = 'http://h.hiphotos.baidu.com/baike/w%3d268%3bg%3d0/sign=d11bad13d0160924dc25a51dec3c52c7/faf2b2119313b07ead669a4d0cd7912396dd8c98.jpg';

//$image = 'https://dn-iampkuhz.qbox.me/images/iampkuhz.png';
$imageData = base64_encode(file_get_contents($image));
echo '<img src="data:image/jpeg;base64,'.$imageData.'">';
?>
