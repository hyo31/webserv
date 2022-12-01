#!/usr/bin/perl -wT
use strict;
use CGI qw(:standard);
use diagnostics;

my $outfile = $ENV{'FILE_NAME'};
open(my $fh, ">>", "logs/form.log")
    or die "Can not open file: $!";
my @values = split(/&/,$ENV{'QUERY_STRING'});
my $datestring = localtime();
print $fh "Local date and time $datestring\n";
foreach my $i (@values) {
    my($fieldname, $data) = split(/=/, $i);
    print $fh "$fieldname = $data\n";
}
print $fh "\n";
close($fh);
print <<ENDTAG;
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Webserv</title>
    <link rel="icon" type="image/x-icon" href="/images/favicon.ico">
</head>
<body style="background-color:rgb(67, 67, 67);">
    <h1 style="text-align:center;font-size: 52px"><a href="/index" style="text-decoration: none;color: white">WEBSERV</a></h1><br><br>
        <h2 style="text-align:center;color:rgb(255, 255, 255);font-size: 26px;"><a href="/submitform" style="text-decoration: none;color: white">Submit a Form</a></h2><br>
        <h2 style="text-align:center;color:rgb(255, 255, 255);font-size: 26px;"><a href="/uploadfile" style="text-decoration: none;color: white">Upload a File</a></h2><br>
</body>
</html>
ENDTAG