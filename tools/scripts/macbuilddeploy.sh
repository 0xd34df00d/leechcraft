#! /bin/sh

TARGET=/usr/local

BASEDIR=$(dirname $0)
echo $BASEDIR

sudo rm -rf $TARGET/leechcraft.app/Contents/Resources/qt.conf $TARGET/leechcraft.app/Contents/Frameworks $TARGET/leechcraft.app/Contents/PlugIns
make -j8 install
cp -Rv /usr/local/Cellar/qt/*/plugins/* $TARGET/leechcraft.app/Contents/PlugIns
mkdir $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
install -v /usr/local/lib/gstreamer-1.0/* $TARGET/leechcraft.app/Contents/PlugIns/gstreamer
install -v /usr/local/lib/libgst* $TARGET/leechcraft.app/Contents/Frameworks
sudo macdeployqt $TARGET/leechcraft.app -verbose=2
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
#DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 $TARGET/leechcraft.app/Contents/MacOs/leechcraft
