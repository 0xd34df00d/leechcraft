#! /bin/sh

# $Id: svnlogxml.sh 2394 2008-06-08 13:22:00Z roman_rybalko $

rev=$1
[ -z "$rev" ] && { echo "USAGE: $0 <from_revision>"; exit 1; }
newrev=$2
[ -z "$newrev" ] && newrev=head

exec svn log -r $newrev:$rev --xml ..
