rem == General variables setup script ==
rem == (c) Eugene Mamin <thedzhon@gmail.com>

@echo off

rem == Build variables ==

call "C:\Program Files\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=C:\Qt\4.7.4

set BOOST_ROOT=C:\boost
set TORRENT_DIR=C:\DEVLIBS\libtorrent-rasterbar-0.15.7
set QXMPP_LOCAL=C:\DEVLIBS\qxmpp-dev
set QJSON_DIR=C:\DEVLIBS\qjson
set SPEEX_DIR=C:\DEVLIBS\speex-1.2rc1

set BUILD_RELEASE_AND_DEBUG=1

rem == Collect variables ==

set BUILD_TYPE=Debug

rem Set these variables to proper paths of your system:

set BOOST_BIN_DIR="%BOOST_ROOT%\stage\lib"
set BOOST_VERSION="1_47"
set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\msvc-10.0\debug\boost-link-shared\boost-source\threading-multi"
set OPENSSL_BIN_DIR="C:\DEVLIBS\OpenSSL-Win32"
set QT_BIN_DIR="%QTDIR%\bin"
set QJSON_BIN_DIR="%QJSON_DIR%\build\lib\Debug\"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"

