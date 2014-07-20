#! /bin/bash

stripped=$(/usr/bin/basename $1 .xml)
stylepath=`dirname $0`
xsltproc $stylepath/transform.xsl $1 | grep QT_TRANSL | sed s/__FILENAME__/$stripped/g >> dummy.cpp
