#! /bin/sh

set -e

for i in `ls base`; do docker build --force-rm --no-cache -t leechcraft/ci_$i:latest base/$i && docker push leechcraft/ci_$i:latest; done
