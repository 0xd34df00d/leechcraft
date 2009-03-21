#! /usr/bin/perl -w

# $Id: svnlogxml2changelog.pl 2394 2008-06-08 13:22:00Z roman_rybalko $

use strict;

foreach(join('',<>) =~ m~<msg>(.+?)</msg>~sg)
{
	s/^\s*$//mg; # remove empty lines
	s/\n$//s; # remove the last LF
	s/\n/\n    /g; # handle multiline comments
	print "  * ";
	print;
	print "\n";
}
