#! /bin/sh

# Intended to run from src/, collects all the POs into the out subdir.

rm -rf out
mkdir out

mkdir out/src
for i in `ls leechcraft_*.ts`; do ts2po -i $i -o out/src/$(echo $i | cut -b 12-13).po; done
rm -f pot.ts
lupdate *.cpp *.h *.ui plugininterface/*.cpp plugininterface/*.h plugininterface/*.ui xmlsettingsdialog/*.cpp xmlsettingsdialog/*.h -ts pot.ts
ts2po -i pot.ts -o out/src/src.pot -P

for plugin in aggregator anhero auscrie bittorrent cstp dbusmanager deadlyrics historyholder kinotify lcftp lmp networkmonitor newlife poshuku seekthru summary tabpp vgrabber; do
	echo "Processing $plugin..."
	plugsize=$(echo plugins/$plugin/leechcraft_$plugin | wc -c)
	mkdir out/$plugin
	for tsfile in `ls plugins/$plugin/leechcraft_${plugin}_*.ts`; do
		ts2po -i $tsfile -o out/$plugin/$(echo $tsfile | cut -b $((1+$plugsize))-$((2+$plugsize))).po;
	done
	rm -f pot.ts
	lupdate plugins/$plugin/*.cpp plugins/$plugin/*.h plugins/$plugin/*.ui -ts pot.ts || lupdate plugins/$plugin/*.cpp plugins/$plugin/*.h -ts pot.ts
	ts2po -i pot.ts -o out/$plugin/$plugin.pot -P
	rm pot.ts
done

for plugin in cleanweb filescheme fua wyfv; do
	echo "Processing Poshuku $plugin..."
	plugsize=$(echo plugins/poshuku/plugins/$plugin/leechcraft_poshuku_$plugin | wc -c)
	mkdir out/poshuku_$plugin
	for tsfile in `ls plugins/poshuku/plugins/$plugin/leechcraft_poshuku_${plugin}_*.ts`; do
		ts2po -i $tsfile -o out/poshuku_$plugin/$(echo $tsfile | cut -b $((1+$plugsize))-$((2+$plugsize))).po;
	done
	rm -f pot.ts
	lupdate plugins/poshuku/plugins/$plugin/*.cpp plugins/poshuku/plugins/$plugin/*.h plugins/poshuku/plugins/$plugin/*.ui -ts pot.ts || lupdate plugins/poshuku/plugins/$plugin/*.cpp plugins/poshuku/plugins/$plugin/*.h -ts pot.ts
	ts2po -i pot.ts -o out/poshuku_$plugin/poshuku_$plugin.pot -P
	rm pot.ts
done
