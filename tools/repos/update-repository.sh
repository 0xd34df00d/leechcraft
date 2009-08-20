#!/bin/sh

FTPARCHIVE="/home/d34df00d/Programming/apt/apt-0.7.20.2/bin/apt-ftparchive"

LD_LIBRARY_PATH=/home/d34df00d/Programming/apt/apt-0.7.20.2/bin ${FTPARCHIVE} generate apt-ftparchive.conf

LD_LIBRARY_PATH=/home/d34df00d/Programming/apt/apt-0.7.20.2/bin ${FTPARCHIVE} -c apt-master-release.conf \
	release dists/master > dists/master/Release
LD_LIBRARY_PATH=/home/d34df00d/Programming/apt/apt-0.7.20.2/bin ${FTPARCHIVE} -c apt-0.3-release.conf \
	release dists/0.3 > dists/0.3/Release

for file in `find . -name Release` ; do
	rm -f "${file}.gpg"
	gpg -abs -u 43CAD081 -o "${file}.gpg" "${file}"
done
