#! /bin/sh

find -name 'Contents*' -exec rm -f {} \;
find -name 'Packages*' -exec rm -f {} \;
find -name 'Sources*' -exec rm -f {} \;
find -name 'Release*' -exec rm -f {} \;
rm -f *.db
