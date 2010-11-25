#!/bin/sh
FILENAME=${BASH_SOURCE[0]}
cd ${FILENAME%/*}
pwd > ~/text.txt
./leechcraft
