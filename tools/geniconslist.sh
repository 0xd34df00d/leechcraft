#! /bin/sh

grep ActionIcon *
for i in `ls plugins`; do echo "List for $i:"; grep ActionIcon plugins/$i/*; done
