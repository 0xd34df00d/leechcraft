#! /bin/sh

# Intended to run from src/.

mkdir out/src
lupdate *.cpp *.h *.ui plugininterface/*.cpp plugininterface/*.h plugininterface/*.ui xmlsettingsdialog/*.cpp xmlsettingsdialog/*.h -ts *.ts

for plugin in aggregator anhero auscrie bittorrent cstp dbusmanager deadlyrics historyholder kinotify lcftp lmp networkmonitor newlife poshuku seekthru summary tabpp vgrabber; do
	echo "Processing $plugin..."
	lupdate plugins/$plugin/*.cpp plugins/$plugin/*.h plugins/$plugin/*.ui -ts plugins/$plugin/*.ts || lupdate plugins/$plugin/*.cpp plugins/$plugin/*.h -ts plugins/$plugin/*.ts
done

for plugin in cleanweb filescheme fua wyfv; do
	echo "Processing Poshuku $plugin..."
	lupdate plugins/poshuku/plugins/$plugin/*.cpp plugins/poshuku/plugins/$plugin/*.h plugins/poshuku/plugins/$plugin/*.ui -ts plugins/poshuku/plugins/$plugin/*.ts || lupdate plugins/poshuku/plugins/$plugin/*.cpp plugins/poshuku/plugins/$plugin/*.h -ts plugins/poshuku/plugins/$plugin/*.ts
done
