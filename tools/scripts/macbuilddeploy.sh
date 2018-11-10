#! /bin/sh

set -e
set -x

TARGET=/usr/local
QTVERSION=qt
DYLDINFO=/Library/Developer/CommandLineTools/usr/bin/dyldinfo
ICONPACK=../../oxygen

BASEDIR=$(dirname $0)
echo $BASEDIR

rm -rf $TARGET/leechcraft.app/Contents/Resources/qt.conf $TARGET/leechcraft.app/Contents/Frameworks $TARGET/leechcraft.app/Contents/PlugIns
make -j8 install

mkdir -p $TARGET/leechcraft.app/Contents/Resources/icons/
cp -R $ICONPACK $TARGET/leechcraft.app/Contents/Resources/icons/

cp -LRv /usr/local/Cellar/$QTVERSION/*/plugins/* $TARGET/leechcraft.app/Contents/PlugIns
#mkdir $TARGET/leechcraft.app/Contents/PlugIns/quick
#cp -LRv /usr/local/Cellar/$QTVERSION/*/qml/*/*/*.dylib $TARGET/leechcraft.app/Contents/PlugIns/quick
#cp -LRv /usr/local/Cellar/$QTVERSION/*/qml/*/*.dylib $TARGET/leechcraft.app/Contents/PlugIns/quick
find $TARGET/leechcraft.app/Contents/PlugIns -type f -exec chmod 644 '{}' \;

for SUB in QtGraphicalEffects QtQuick.2; do
	mkdir -p $TARGET/leechcraft.app/Contents/Resources/qml/$SUB
	cp -Lv /usr/local/opt/qt/qml/$SUB/*dylib $TARGET/leechcraft.app/Contents/Resources/qml/$SUB
	chmod 644 $TARGET/leechcraft.app/Contents/Resources/qml/$SUB/*dylib
done

cp -Lv /usr/local/lib/libgst* $TARGET/leechcraft.app/Contents/Frameworks
chmod 644 $TARGET/leechcraft.app/Contents/Frameworks/libgst*

mkdir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
cp -Lv /usr/local/lib/gstreamer-1.0/lib* $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
cp -Lv /usr/local/lib/gio/modules/*.so $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
chmod 644 $TARGET/leechcraft.app/Contents/PlugIns/gstreamer/*

cp -LRv /usr/local/share/glib-2.0/schemas $TARGET/leechcraft.app/Contents/Frameworks

$BASEDIR/replacepath.py --old /usr/local/lib/libgstreamer --new @executable_path/../Frameworks/libgstreamer --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/lib/libgobject --new @executable_path/../Frameworks/libgobject --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/lib/libglib --new @executable_path/../Frameworks/libglib --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/lib/libgmodule --new @executable_path/../Frameworks/libgmodule --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/lib/libgst --new @executable_path/../Frameworks/libgst --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/Cellar/gst-plugins-good/*/lib/ --new @executable_path/../Frameworks/ --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/Cellar/gst-plugins-base/*/lib/ --new @executable_path/../Frameworks/ --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/Cellar/gst-plugins-bad/*/lib/ --new @executable_path/../Frameworks/ --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/Cellar/gst-plugins-ugly/*/lib/ --new @executable_path/../Frameworks/ --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/Cellar/gstreamer/*/lib/ --new @executable_path/../Frameworks/ --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
$BASEDIR/replacepath.py --old /usr/local/opt/gettext/lib/ --new @executable_path/../Frameworks/ --dir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
for GST in $TARGET/leechcraft.app/Contents/PlugIns/gstreamer/lib*.so; do
	for LIB in `$DYLDINFO -dylibs $GST | grep -v executable_path | grep -v attributes | grep -v System | grep -v /usr/lib`; do
		echo Library $LIB
		install -v $LIB $TARGET/leechcraft.app/Contents/Frameworks
		install_name_tool -change $LIB @executable_path/../Frameworks/$(basename $LIB) $GST
	done
done

#Kludge for macdeployqt not handling @loader_path-prefixed dependencies.
install -v /usr/local/lib/libboost_chrono*dylib $TARGET/leechcraft.app/Contents/Frameworks/

cp -LRv /usr/local/opt/qca/lib/qt5/plugins/crypto $TARGET/leechcraft.app/Contents/PlugIns
chmod 644 $TARGET/leechcraft.app/Contents/PlugIns/crypto/*
CDIR=$TARGET/leechcraft.app/Contents/PlugIns/crypto; for PLUG in `ls $CDIR`; do install_name_tool -change $($DYLDINFO -dylibs $CDIR/$PLUG | grep qca) @executable_path/../Frameworks/qca.framework/qca $CDIR/$PLUG; done

macdeployqt $TARGET/leechcraft.app -verbose=2 -executable=$TARGET/leechcraft.app/Contents/MacOs/lc_anhero_crashprocess -qmldir=$TARGET/leechcraft.app/Contents/Resources/share/qml5

cd $BASEDIR
cat Makefile.in | sed s/LC_VERSION/$(git describe)/ > Makefile
make
#DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 $TARGET/leechcraft.app/Contents/MacOs/leechcraft
