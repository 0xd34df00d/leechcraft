#! /bin/sh

TARGET=/usr/local

sudo rm -rf $TARGET/leechcraft.app/Contents/Resources/qt.conf $TARGET/leechcraft.app/Contents/Frameworks $TARGET/leechcraft.app/Contents/PlugIns
make -j8 install
cp -Rv /usr/local/Cellar/qt/*/plugins/* $TARGET/leechcraft.app/Contents/PlugIns
cd $TARGET
sudo macdeployqt leechcraft.app -verbose=2
DYLD_PRINT_LIBRARIES_POST_LAUNCH=1 leechcraft.app/Contents/MacOs/leechcraft
