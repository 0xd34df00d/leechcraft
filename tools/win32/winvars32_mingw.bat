rem == General variables setup script ==
rem == (c) Oleg Linkin <MaledictusDeMagog@gmail.com>

@echo off

rem == Build variables ==

set QTDIR=c:\DEVLIBS\QtSDK\Desktop\Qt\4.8.2_sources_gcc

set BOOST_ROOT=c:\DEVLIBS\boost_1_49_gcc
set QJSON_DIR=C:\DEVLIBS\qjson
set TAGLIB_DIR=C:\DEVLIBS\taglib-1.7.2
set QCA2_DIR=C:\DEVLIBS\qca-2.0.1-mingw
set SPEEX_DIR=c:\DEVLIBS\speex-1.2rc1/build
set QXMPP_DIR=C:\DEVLIBS\qxmpp-dev
set QWT_DIR=C:\DEVLIBS\qwt-6.0.1
set TORRENT_DIR=c:\DEVLIBS\libtorrent-rasterbar-0.16.3
set POPPLER_DIR=c:\DEVLIBS\poppler-0.20.3

set BUILD_RELEASE_AND_DEBUG=1

rem == Collect variables ==

set BUILD_TYPE=Release

rem Set these variables to proper paths of your system:

set BOOST_BIN_DIR="%BOOST_ROOT%\stage\lib"
set BOOST_VERSION="1_49"
set OPENSSL_BIN_DIR="C:\DEVLIBS\OpenSSL-Win32"
set QT_BIN_DIR="%QTDIR%\bin"
set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\gcc-mingw-4.7.0\release\boost-link-shared\boost-source\encryption-off\threading-multi"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"