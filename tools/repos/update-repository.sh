#!/bin/sh

apt-ftparchive generate apt-ftparchive.conf

apt-ftparchive -c apt-snapshots-release.conf \
	release dists/snapshots > dists/snapshots/Release
apt-ftparchive -c apt-releases-release.conf \
	release dists/releases > dists/releases/Release

for file in `find . -name Release` ; do
	rm -f "${file}.gpg"
	gpg -abs -u 43CAD081 -o "${file}.gpg" "${file}"
done
