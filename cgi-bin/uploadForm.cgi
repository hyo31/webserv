#!/usr/bin/perl -wT
use strict;
use CGI qw(:standard);

print <<ENDTAG;
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Webserv</title>
    <link rel="icon" type="image/x-icon" href="/images/favicon.ico">
</head>
<body style="background-color:rgb(67, 67, 67);">
    <h1 style="text-align:center;font-size: 52px"><a href="/index" style="text-decoration: none;color: white">WEBSERV</a></h1><br><br>
        <h2 style="text-align:center;color:rgb(255, 255, 255);font-size: 26px;"><a href="/uploadform" style="text-decoration: none;color: white">Submit a Form</a></h2><br>
        <h2 style="text-align:center;color:rgb(255, 255, 255);font-size: 26px;"><a href="/uploadfile" style="text-decoration: none;color: white">Upload a File</a></h2><br>
        <h2 style="text-align:center;color:rgb(255, 255, 255);font-size: 26px;"><a href="/doesntexist" style="text-decoration: none;color: white">test3</a></h2>
</body>
</html>
ENDTAG