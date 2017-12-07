#!/usr/bin/perl
use strict;
use warnings;

# for setting timezone
use POSIX qw(tzset);

# helper variables
my $file = "build.cpp";
my $numPrefix = "NUMBER =";
my $timePrefix = "TIME =";

# read entire file
open( my $in, "<", $file );

undef $/;
my $contents = <$in>;

# get build number and increment it
my $buildNum = 0;
$buildNum = $1 if $contents =~ /$numPrefix (\d+)/;

$buildNum++;
close $in;

# get the time
$ENV{TZ} = 'America/Los_Angeles';
#tzset; # set the new timezone
my $time = localtime;

# change the contents and output the file
$contents =~ s/$numPrefix (\d+)/$numPrefix $buildNum/;
$contents =~ s/$timePrefix .*$/$timePrefix "$time";/m;

open( my $out, ">", $file );
print $out $contents;
close $out;

