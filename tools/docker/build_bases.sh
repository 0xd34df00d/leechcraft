#! /bin/sh

for i in `ls base`; do docker build --force-rm --no-cache -t leechcraft/ci_$i:latest $i; docker push leechcraft/ci_$i:latest; done
