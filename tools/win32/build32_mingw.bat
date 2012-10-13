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
	 %BTYPE% -DENABLE_DBUSMANAGER=False -DENABLE_ANHERO=False ^
	-DENABLE_LASTFM=False -DENABLE_LADS=False -DENABLE_VROOBY=False ^
	-DENABLE_AZOTH_ROSENTHAL=False ^
	-DENABLE_LEMON=True -DENABLE_OTLOZHU=True -DENABLE_BLOGIQUE=True ^
	-DQJSON_DIR=%QJSON_DIR% ^
	-DTAGLIB_DIR=%TAGLIB_DIR% ^
	-DQCA2_DIR=%QCA2_DIR% ^
	-DSPEEX_DIR=%SPEEX_DIR% ^
	-DQXMPP_LOCAL=%QXMPP_DIR% ^
	-DQWT_DIR=%QWT_DIR% ^
	-DRBTorrent_DIR=%TORRENT_DIR% ^
	-DPOPPLER_DIR=%POPPLER_DIR%

pause