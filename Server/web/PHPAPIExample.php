<?php
printf("this is");

$result = $_POST["queryAns1"];
$format = $_POST["format"];
switch ($format) {
case 1:
    $array = explode("<", $result);
    $html = '<html><table class="sparql" border="1"><tr><th>' . $array[0] . "</th></tr>";
    for ($i = 1; $i < count($array); $i++) {
        $href = str_replace(">", "", $array[$i]);
        $html.= '<tr><td><a href="' . $href . '">' . $href . '</a></td></tr>';
    }
    $html.= '</table></html>';
    echo $html;
    exit;

case 2:
    $filename = 'sparql.txt';
    header("Content-Type: application/octet-stream");
    header('Content-Disposition: attachment; filename="' . $filename . '"');
    echo $result;
    exit;

case 3:
    $filename = 'sparql.csv';
    header("Content-Type: application/octet-stream");
    header('Content-Disposition: attachment; filename="' . $filename . '"');
    $array = explode("<", $result);
    echo $array[0];
    for ($i = 1; $i < count($array); $i++) {
        $href = str_replace(">", "", $array[$i]);
        echo $href;
    }
    exit;
}
?>
