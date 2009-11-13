@echo off
rem === Set all these variables to proper paths of your system:
call "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\vcvarsall.bat" x86

set QTDIR=c:\Programs\qt-win-opensource-src-4.5.3
set BOOST_ROOT=f:\X-Files\Projects\Lib\boost_1_40_0

set TORRENT_DIR=F:/X-Files/Projects/Lib/libtorrent
set TORRENT_INCLUDE_DIR=%TORRENT_DIR%/include
set TORRENT_LIBRARY=%TORRENT_DIR%/bin/msvc-9.0/release/boost-link-shared/boost-source/threading-multi/torrent.lib
set CURL_INLUDE_DIR=F:/X-Files/Projects/Lib/curl-7.19.6/include
set CURL_LIBRARY=F:/X-Files/Projects/Lib/curl-7.19.6/lib/DLL-Release/libcurl_imp.lib

rem BZIP2 libraries are not used for now, but they have to stay here due to cmake's stupidness.
set BZIP2_INCLUDE_DIR=F:/X-Files/Projects/Lib/bzip2-1.0.5/
set BZIP2_LIBRARY=F:/X-Files/Projects/bzip2-1.0.5/Release/libbz2.lib

rem Be sure that cmake executable is in your system %PATH%.
if exist build32 del /f /s /q build32
if not exist build32 mkdir build32
cd build32
cmake ../../../src -G "Visual Studio 9 2008" -DCMAKE_BUILD_TYPE=Release -DRBTorrent_INCLUDE_DIR=%TORRENT_INCLUDE_DIR% -DRBTorrent_LIBRARY=%TORRENT_LIBRARY% -DCURL_INCLUDE_DIR=%CURL_INLUDE_DIR% -DCURL_LIBRARY=%CURL_LIBRARY% -DBZIP2_INCLUDE_DIR=%BZIP2_INCLUDE_DIR% -DBZIP2_LIBRARIES=%BZIP2_LIBRARY%
pause