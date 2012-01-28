rem == General variables setup script ==
rem == (c) Eugene Mamin <thedzhon@gmail.com>

@echo off

rem == Build variables ==

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=C:\Qt\4.8.0

set BOOST_ROOT=C:\boost
set TORRENT_DIR=C:\DEVLIBS\libtorrent-rasterbar-0.15.9
set QXMPP_LOCAL=C:\DEVLIBS\qxmpp-dev
set QJSON_DIR=C:\DEVLIBS\qjson-0.7.1
set SPEEX_DIR=C:\DEVLIBS\speex-1.2rc1
rem Go to http://wiki.videolan.org/GenerateLibFromDll first!
rem Generate libs into this folder
set VLC_DIR=C:\DEVLIBS\vlc-1.1.11-win32\vlc-1.1.11
set QWT_DIR=C:\DEVLIBS\qwt-6.0.1

set BUILD_RELEASE_AND_DEBUG=1

rem == Collect variables ==

set BUILD_TYPE=Release

rem Set these variables to proper paths of your system:

set BOOST_BIN_DIR="%BOOST_ROOT%\stage\lib"
set BOOST_VERSION="1_48"
set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\msvc-10.0\Release\boost-link-shared\boost-source\threading-multi"
set OPENSSL_BIN_DIR="C:\DEVLIBS\OpenSSL-Win32"
set QT_BIN_DIR="%QTDIR%\bin"
set QJSON_BIN_DIR="%QJSON_DIR%\build\lib\MinSizeRel\"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"

