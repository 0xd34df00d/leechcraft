#! /bin/sh

# $Id: do_changelog.sh 2396 2008-06-08 14:03:37Z roman_rybalko $

oldrev=$1
newrev=$2

[ -z "$newrev" -o -z "$oldrev" ] && { echo "USAGE: $0 <oldrev> <newrev>"; exit 1; }

set -e
set -x

CHLOG=changelog.new
echo "leechcraft (`git describe`) unstable; urgency=low" > $CHLOG
echo >> $CHLOG
git log --pretty=format:'  %s' | sed -r "s/\. \+/\.\n  \+/" | sed -r "s/\. \*/\.\n  \*/" | sed -r "s/\> \*/\n  \*/" | sed -r "s/\. \-/\.\n  \-/" | sed -r "s/(.{1,72})(.+)/\1/" | grep "[:alphanum:]\+" >> $CHLOG
echo >> $CHLOG
echo -n " -- Rudoy Georg <0xd34df00d@gmail.com>  " >> $CHLOG
date -R >> $CHLOG
echo >> $CHLOG
cat changelog >> $CHLOG

mv -f $CHLOG changelog
