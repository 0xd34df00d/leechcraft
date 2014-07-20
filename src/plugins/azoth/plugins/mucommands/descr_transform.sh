#! /bin/sh

xsltproc descr_transform.xsl "resources/data/descriptions.xml" | grep QT_TRANSL | sed s/__FILENAME__/descriptions/g >> dummy.cpp
