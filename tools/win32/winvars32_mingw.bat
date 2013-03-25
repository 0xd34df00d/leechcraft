rem == General variables setup script ==
rem == (c) Oleg Linkin <MaledictusDeMagog@gmail.com>

@echo off

rem == Build variables ==

set QTDIR=C:\Dev\qt-everywhere-opensource-src-4.8.4

set BOOST_ROOT=C:\Installed\boost-1.53
set QJSON_DIR=C:\Installed\qjson
set TAGLIB_DIR=C:\Installed\taglib-1.8
set QCA2_DIR=C:\Installed\qca
set SPEEX_DIR=C:\Installed\speex-1.2rc1
set QXMPP_DIR=C:\Installed\qxmpp-0.7.6
set QWT_DIR=C:\Installed\Qwt-6.1.0-svn
set TORRENT_DIR=C:\Dev\libtorrent-trunk
set PCRE_DIR=C:\Installed\pcre-8.21
set POPPLER_DIR=C:\Dev\poppler-0.20.3
set LIBLASTFM_DIR=C:\Installed\liblastfm
set HUNSPELL_DIR=C:\Installed\hunspell-1.3.2
set CMAKE_PREFIX_PATH=%BOOST_ROOT%;%BOOST_ROOT%;%QJSON_DIR%;%TAGLIB_DIR%;%QCA2_DIR%;%SPEEX_DIR%;%QXMPP_DIR%;%QWT_DIR%;%TORRENT_DIR%;%PCRE_DIR%;%POPPLER_DIR%;%LIBLASTFM_DIR%;%HUNSPELL_DIR%

rem 7za.exe, gunzip.exe, vcredist_x86.exe, myspell dicts
set TOOLS_DIR=C:\Installed\tools

set BUILD_RELEASE_AND_DEBUG=1

rem == Collect variables ==

set BUILD_TYPE=Release

rem This is the directory where LeechCraft will live
set TARGET_DIR=LeechCraft
set SIGNCOMMAND=signtool sign /a /t http://time.certum.pl
set SIGNCODE=1
rem Set these variables to proper paths of your system:

set BOOST_BIN_DIR="%BOOST_ROOT%\lib"
set BOOST_VERSION="1_53"
set OPENSSL_BIN_DIR="C:\Installed\OpenSSL-Win32"
set QT_BIN_DIR="%QTDIR%\bin"
set LIBTORRENT_BIN_DIR="%TORRENT_DIR%\bin\gcc-mingw-4.7.2\release\boost-link-shared\boost-source\iconv-on\threading-multi"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"