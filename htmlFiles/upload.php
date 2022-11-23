<?php
$myfile = fopen("test.txt", "w") or die("Unable to open file!");
$txt = "test123\n";
fwrite($myfile, $txt);
fclose($myfile);
header('Location:'.$_POST['goback']);
exit();
?>