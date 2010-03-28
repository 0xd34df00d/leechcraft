@echo off
rem === Set all these variables to proper paths of your system:
call "C:\Program Files\Microsoft Visual Studio 9.0\VC\bin\vcvars32.bat"

set QTDIR=c:\Qt\4.6.2

set BOOST_ROOT=C:\Boost\1.42

set TORRENT_DIR=C:/Programming/libtorrent/
set TORRENT_INCLUDE_DIR=%TORRENT_DIR%/include
set TORRENT_LIBRARY=%TORRENT_DIR%/bin/msvc-9.0/release/boost-link-shared/boost-source/threading-multi/torrent.lib

set CURL_INLUDE_DIR=C:/Programming/curl/curl-7.20.0/include
set CURL_LIBRARY=C:/Programming/curl/curl-7.20.0/lib/Release/libcurl.lib

rem BZIP2 libraries are not used for now, but they have to stay here due to cmake's stupidness.
rem set BZIP2_INCLUDE_DIR=F:/X-Files/Projects/Lib/bzip2-1.0.5/
rem set BZIP2_LIBRARY=F:/X-Files/Projects/bzip2-1.0.5/Release/libbz2.lib

rem set PYTHONDIR="c:/Program Files (x86)/Python26"
rem set PYTHON_INCLUDE_DIR=PYTHONDIR/include
rem set PYTHON_LIBRARY=PYTHONDIR/libs/python26.lib

rem Be sure that cmake executable is in your system %PATH%.
if exist build32 del /f /s /q build32
if not exist build32 mkdir build32
cd build32
rem cmake ../../../src -G "Visual Studio 9 2008" -DCMAKE_BUILD_TYPE=Release -DRBTorrent_INCLUDE_DIR=%TORRENT_INCLUDE_DIR% -DRBTorrent_LIBRARY=%TORRENT_LIBRARY% -DCURL_INCLUDE_DIR=%CURL_INLUDE_DIR% -DCURL_LIBRARY=%CURL_LIBRARY% -DBZIP2_INCLUDE_DIR=%BZIP2_INCLUDE_DIR% -DBZIP2_LIBRARIES=%BZIP2_LIBRARY% -DPYTHON_INCLUDE_DIR=%PYTHON_INCLUDE_DIR% -DPYTHON_LIBRARY=%PYTHON_LIBRARY%
cmake ../../../src -G "Visual Studio 9 2008" -DCMAKE_BUILD_TYPE=Release -DRBTorrent_INCLUDE_DIR=%TORRENT_INCLUDE_DIR% -DRBTorrent_LIBRARY=%TORRENT_LIBRARY% -DCURL_INCLUDE_DIR=%CURL_INLUDE_DIR% -DCURL_LIBRARY=%CURL_LIBRARY% -DENABLE_DCMINATOR=False -DENABLE_PYLC=False
pause