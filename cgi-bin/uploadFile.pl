#!/usr/bin/perl
use strict;
use CGI qw(:standard);
use diagnostics;

my $outfile = $ENV{'FILE_NAME'};
my @a = (1..10);
my $filename = $outfile;
my $og_outfile = $outfile;
for(@a){
    if (-e "uploads/".$filename){
        $outfile = $og_outfile.$_;     #file exists
    }
    $filename = $og_outfile.$_;
}
open(my $fh, ">>", "uploads/".$outfile)
    or die "Can not open file: $!";
my $content = $ENV{'FILE_BODY'};
print $fh "$content";
open(my $rf, ">>", $ENV{'RESPONSE_FILE'})
    or die "Can not open file: $!";
print $rf <<ENDTAG;
<!DOCTYPE html>
<html lang="en">
<head>
    <title>Webserv</title>
    <link rel="icon" type="image/x-icon" href="/images/favicon.ico">
</head>
<body style="background-color:rgb(67, 67, 67);">
    <h1 style="text-align:center;font-size: 52px"><a href="/index" style="text-decoration: none;color: white">WEBSERV</a></h1><br><br>
        <p1 style=";color:rgb(255, 255, 255);font-size: 26px;"><center>Thanks for uploading a file!</p1><br>
        <p1 style=";color:rgb(255, 255, 255);font-size: 26px;">Your file is saved as $outfile</center></p1><br>
</body>
</html>
ENDTAG
close($rf);