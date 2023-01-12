#!/usr/bin/perl
use strict;
use CGI qw(:standard);
use diagnostics;

for( ; ; ) {
   printf "This loop will run forever.\n";
}
my $outfile = $ENV{'FILE_NAME'};
open(my $fh, ">>", "forms/".$outfile)
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
        <p1 style="color:rgb(255, 255, 255);font-size: 26px;"><center>Thanks for submitting a form!</center></p1><br>
</body>
</html>
ENDTAG
close($rf);