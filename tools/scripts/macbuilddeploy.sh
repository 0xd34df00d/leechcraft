#! /bin/sh

TARGET=/usr/local

BASEDIR=$(dirname $0)
echo $BASEDIR

sudo rm -rf $TARGET/leechcraft.app/Contents/Resources/qt.conf $TARGET/leechcraft.app/Contents/Frameworks $TARGET/leechcraft.app/Contents/PlugIns
make -j8 install

# Kludge
install_name_tool -change lib/libqtermwidget4.0.dylib /usr/local/lib/libqtermwidget4.0.5.1.dylib $TARGET/leechcraft.app/Contents/PlugIns/libleechcraft_eleeminator.dylib
sudo cp /usr/local/leechcraft.app/Contents/Frameworks/libleechcraft-* /usr/lib

cp -Rv /usr/local/Cellar/qt/*/plugins/* $TARGET/leechcraft.app/Contents/PlugIns
mkdir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
install -v /usr/local/lib/gstreamer-1.0/* $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
install -v /usr/local/lib/libgst* $TARGET/leechcraft.app/Contents/Frameworks
install -v /usr/local/lib/gio/modules/*.so $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
cp -Rv /usr/local/share/glib-2.0/schemas $TARGET/leechcraft.app/Contents/Frameworks
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
	for LIB in `dyldinfo -dylibs $GST | grep -v executable_path | grep -v attributes | grep -v System | grep -v /usr/lib`; do
		echo Library $LIB
		cp $LIB $TARGET/leechcraft.app/Contents/Frameworks
		install_name_tool -change $LIB @executable_path/../Frameworks/$(basename $LIB) $GST
	done
done
sudo macdeployqt $TARGET/leechcraft.app -verbose=2

# Kludge
sudo rm /usr/lib/libleechcraft-*

cd $BASEDIR
cat Makefile.in | sed s/LC_VERSION/$(git describe)/ > Makefile
make
#DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 $TARGET/leechcraft.app/Contents/MacOs/leechcraft
