#! /bin/sh

grcmd='grep -E "(ActionIcon|\<GetIcon)"'

touch tmp
grep ActionIcon * | sed 's/.*\".*\".*\"\(.*\)\".*/\1/' > tmp2
grep "GetIcon (\"" * | sed 's/.*GetIcon (\"\(.*\)\".*)/\1/' >> tmp2
echo "List for Core:" >> tmp
sort -u tmp2 >> tmp
for i in `ls plugins`;
	do
		grep ActionIcon plugins/$i/* | sed 's/.*\".*\".*\"\(.*\)\".*/\1/' > tmp2
		grep "GetIcon (\"" plugins/$i/* | sed 's/.*GetIcon (\"\(.*\)\".*)/\1/' >> tmp2
#		if [ -n `cat $tmp2` ]
#		then
			echo "List for $i:" >> tmp
			sort -u tmp2 >> tmp
#		fi
	done


cat tmp
rm tmp2
rm tmp
