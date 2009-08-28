#! /bin/sh

touch tmp
grep ActionIcon * > tmp
for i in `ls plugins`; do echo "List for $i:" >> tmp; grep ActionIcon plugins/$i/* | sed 's/.*\".*\".*\"\(.*\)\"/\1/' > tmp2; sort -u tmp2 >> tmp; done


cat tmp
rm tmp2
rm tmp
