#! /bin/sh

mkdir ~/Programming/leechcraft/translations/$1/
lupdate *.cpp *.h *.ui -ts ~/Programming/leechcraft/translations/$1/leechcraft_$1.ts
cp *.ts ~/Programming/leechcraft/translations/$1/
cd ~/Programming/leechcraft/translations/$1/
for i in `ls *.ts`; do ts2po -i $i -o "${i%.*}".po; rm $i; done
mv leechcraft_$1.po leechcraft_$1.pot
