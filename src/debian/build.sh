#! /bin/sh

cd ../
fakeroot dpkg-buildpackage -D -tc -j3 -us -uc
