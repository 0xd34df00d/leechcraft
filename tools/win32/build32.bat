rem == generate script for MSVS sln file == 

@echo off

call "%~dp0\winvars32.bat"

@echo on

cd ../..
for /f "tokens=*" %%a in ('git describe') do set LEECHCRAFT_VERSION=%%a
echo Version: %LEECHCRAFT_VERSION%
cd tools\win32

if %BUILD_RELEASE_AND_DEBUG% == 0 (
	if "%BUILD_TYPE%" == "Debug" (
		SET BTYPE=-DCMAKE_BUILD_TYPE=Debug
	)
	if "%BUILD_TYPE%" == "Release" (
		SET BTYPE=-DCMAKE_BUILD_TYPE=Release
	)
)	

rem Be sure that cmake executable is in your system %PATH%.
if exist build32 rmdir /s /q build32
if not exist build32 mkdir build32
cd build32
cmake ../../../src  ^
	-DENABLE_FTP=False %BTYPE% -DENABLE_DBUSMANAGER=False -DENABLE_ANHERO=False ^
	-DENABLE_LACKMAN=True -DENABLE_SECMAN=True -DENABLE_AZOTH=True ^
	-DENABLE_SHELLOPEN=True -DENABLE_GLANCE=True -DENABLE_TABSLIST=True ^
	-DENABLE_GMAILNOTIFIER=True -DENABLE_ADVANCEDNOTIFICATIONS=True ^
	-DRBTorrent_DIR=%TORRENT_DIR% ^
	-DQXMPP_LOCAL=%QXMPP_LOCAL% ^
	-DQJSON_DIR=%QJSON_DIR% ^
	-DSPEEX_DIR=%SPEEX_DIR% ^
	-DLEECHCRAFT_VERSION=%LEECHCRAFT_VERSION%
pause