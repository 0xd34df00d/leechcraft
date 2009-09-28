#!/bin/sh

FTPARCHIVE="apt-ftparchive"

${FTPARCHIVE} generate apt-ftparchive.conf

${FTPARCHIVE} -c apt-master-release.conf \
	release dists/master > dists/master/Release
${FTPARCHIVE} -c apt-0.3-release.conf \
	release dists/0.3 > dists/0.3/Release

for file in `find . -name Release` ; do
	rm -f "${file}.gpg"
	gpg -abs -u 43CAD081 -o "${file}.gpg" "${file}"
done
