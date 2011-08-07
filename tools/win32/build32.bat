@echo off
rem === Set all these variables to proper paths of your system:
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\vcvars32.bat" x86

set QTDIR=E:\Libs\Qt\4.8.0-b1

set BOOST_ROOT=E:\Libs\boost_1_47_0
set TORRENT_DIR=E:\Libs\libtorrent
set TORRENT_INCLUDE_DIR=%TORRENT_DIR%\include
set TORRENT_LIBRARY=%TORRENT_DIR%\bin\msvc-10.0\release\boost-link-shared\boost-source\threading-multi\torrent.lib
set QXMPP_LOCAL=E:\Libs\qxmpp-dev
set QJSON_DIR=E:\Libs\qjson
set SPEEX_LIB=E:\Libs\speex\lib\libspeex.lib
set SPEEX_INCLUDE=E:\Libs\speex\include

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
cmake ../../../src -DCMAKE_BUILD_TYPE=Release ^
	-DENABLE_FTP=False -DENABLE_DBUSMANAGER=False -DENABLE_ANHERO=False ^
	-DENABLE_LACKMAN=True -DENABLE_SECMAN=True -DENABLE_AZOTH=True ^
	-DENABLE_SHELLOPEN=True -DENABLE_GLANCE=True -DENABLE_TABSLIST=True ^
	-DENABLE_GMAILNOTIFIER=True -DENABLE_ADVANCEDNOTIFICATIONS=True ^
	-DRBTorrent_INCLUDE_DIR=%TORRENT_INCLUDE_DIR% -DRBTorrent_LIBRARY=%TORRENT_LIBRARY% ^
	-DQXMPP_LOCAL=%QXMPP_LOCAL% ^
	-DQJSON_INCLUDE_DIR=%QJSON_DIR% -DQJSON_LIBRARIES=%QJSON_DIR%\build\lib\MinSizeRel\qjson.lib ^
	-DSPEEX_LIBRARIES=%SPEEX_LIB% -DSPEEX_INCLUDE_DIRS=%SPEEX_INCLUDE% ^
	-DLEECHCRAFT_VERSION=%LEECHCRAFT_VERSION%
pause