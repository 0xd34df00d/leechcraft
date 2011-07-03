@echo off
rem === Set all these variables to proper paths of your system:
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=c:\Programs\Qt\qt-everywhere-opensource-src-4.7.3

set BOOST_ROOT=f:\X-Files\Projects\Lib\boost_1_46_1
set TORRENT_DIR=f:\X-Files\Projects\Lib\libtorrent-rasterbar-0.15.6
set TORRENT_INCLUDE_DIR=%TORRENT_DIR%\include
set TORRENT_LIBRARY=%TORRENT_DIR%\bin\msvc-10.0\release\boost-link-shared\boost-source\threading-multi\torrent.lib
set QXMPP_DIR=f:\X-Files\Projects\Lib\qxmpp-dev
set QXMPP_INCLUDE_DIR=%QXMPP_DIR%
set QXMPP_LIBRARIES=%QXMPP_DIR%\lib\qxmpp.lib
set QJSON_DIR=f:\X-Files\Projects\Lib\qjson
set SPEEX_LIB=f:\X-Files\Projects\Lib\speex-1.2rc1\lib\libspeex.lib
set SPEEX_INCLUDE=f:\X-Files\Projects\Lib\speex-1.2rc1\include

set PYTHONDIR=c:/Programs/Python27
set PYTHON_INCLUDE_DIR=%PYTHONDIR%/include
set PYTHON_LIBRARY=%PYTHONDIR%/libs/python27.lib

cd ../..
for /f "tokens=*" %%a in ('git describe') do set LEECHCRAFT_VERSION=%%a
echo Version: %LEECHCRAFT_VERSION%
cd tools\win32

rem Be sure that cmake executable is in your system %PATH%.
if exist build32 rmdir /s /q build32
if not exist build32 mkdir build32
cd build32
cmake ../../../src -DCMAKE_BUILD_TYPE=RelWithDebInfo -DENABLE_FTP=False -DENABLE_DBUSMANAGER=False -DENABLE_ANHERO=False -DENABLE_LACKMAN=True -DENABLE_SECMAN=True -DENABLE_AZOTH=True -DENABLE_SHELLOPEN=True -DRBTorrent_INCLUDE_DIR=%TORRENT_INCLUDE_DIR% -DRBTorrent_LIBRARY=%TORRENT_LIBRARY% -DPYTHON_INCLUDE_DIR=%PYTHON_INCLUDE_DIR% -DPYTHON_LIBRARY=%PYTHON_LIBRARY% -DQXMPP_INCLUDE_DIR=%QXMPP_INCLUDE_DIR% -DQXMPP_LIBRARIES=%QXMPP_LIBRARIES% -DQJSON_INCLUDE_DIR=%QJSON_DIR% -DQJSON_LIBRARIES=%QJSON_DIR% -DSPEEX_LIBRARIES=%SPEEX_LIB% -DSPEEX_INCLUDE_DIRS=%SPEEX_INCLUDE% -DLEECHCRAFT_VERSION=%LEECHCRAFT_VERSION% -DBOOST_LIBRARYDIR=f:\X-Files\Projects\Lib\boost_1_46_1\stage\lib
pause