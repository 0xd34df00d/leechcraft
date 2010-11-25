#!/bin/sh

# assuming, that you launched cmake with 
# -DCMAKE_INSTALL_PREFIX:PATH="/Users/$USER/leechcraft/leechcraft_build"

BUILD_DIR=/Users/$USER/leechcraft/leechcraft_build

rm -rf leechcraft.app

cp -r ${BASH_SOURCE%/*}/leechcraft-app-stub                leechcraft.app

cp -r $BUILD_DIR/bin/leechcraft                           leechcraft.app/Contents/MacOS/
cp    $BUILD_DIR/lib/lib*                                 leechcraft.app/Contents/MacOS/

cp -r $BUILD_DIR/include                                  leechcraft.app/Contents/

cp -r $BUILD_DIR/share/icons                              leechcraft.app/Contents/Resources/
cp -r $BUILD_DIR/share/leechcraft/translations            leechcraft.app/Contents/Resources/
cp -r $BUILD_DIR/share/leechcraft/settings                leechcraft.app/Contents/Resources/
cp    $BUILD_DIR/share/leechcraft/icons/*.mapping         leechcraft.app/Contents/Resources/icons/


cp -r $BUILD_DIR/lib/leechcraft/plugins                   leechcraft.app/Contents/
cp -r $BUILD_DIR/share/leechcraft/installed/              leechcraft.app/Contents/


#AHTUNG!!111 Setup your own path to boost dylibs
cp ~/Library/boost-trunk/stage/lib/libboost_*.dylib       leechcraft.app/Contents/MacOS/