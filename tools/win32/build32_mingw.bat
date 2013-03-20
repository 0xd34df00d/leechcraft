rem == generate script for MinGW makefiles file == 

@echo off

call "%~dp0\winvars32_mingw.bat"

@echo on

set GIT="c:\Program Files (x86)\Git\bin\git.exe"

cd ../..
for /f "tokens=*" %%a in ('%GIT% describe') do set LEECHCRAFT_VERSION=%%a
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
cmake ../../../src -G "MinGW Makefiles" ^
	 %BTYPE% ^
	-DCMAKE_INSTALL_PREFIX:PATH="%~dp0\%TARGET_DIR%" ^
	-DENABLE_DBUSMANAGER=False ^
	-DENABLE_ANHERO=False ^
	-DENABLE_MONOCLE=False ^
	-DENABLE_LMP_MPRIS=False ^
	-DENABLE_VROOBY=False ^
	-DENABLE_AZOTH_VADER=False ^
	-DWITH_MUSICZOMBIE_CHROMAPRINT=False ^
	-DENABLE_LEMON=False ^
	-DQJSON_DIR=%QJSON_DIR% ^
	-DTAGLIB_DIR=%TAGLIB_DIR% ^
	-DSPEEX_DIR=%SPEEX_DIR% ^
	-DRBTorrent_DIR=%TORRENT_DIR%
cd ../
pause