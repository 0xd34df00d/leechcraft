@echo off

rem Set these variables to proper paths of your system
set BOOST_BIN_DIR="F:\X-Files\Projects\boost_1_39_0\stage\lib"
set LIBTORRENT_BIN_DIR="F:\X-Files\Projects\libtorrent\bin\msvc-9.0\release\boost-link-shared\boost-source\threading-multi"
set LIBCURL_BIN_DIR="F:\X-Files\Projects\curl-7.19.6\lib\DLL-Release"
set OPENSSL_BIN_DIR="F:\X-Files\Projects\openssl-0.9.8k\out32dll"
set QT_BIN_DIR="C:\Programs\Qt_4.5.2\bin"

set LEECHCRAFT_ROOT_DIR=".."
set LEECHCRAFT_BUILD_DIR="..\build32"
set BUILD_TYPE="Release"

rem This is the directory where LeechCraft will live
set TARGET_DIR="LeechCraft"

rem === DIRECTORY STRUCTURE ===
if exist %TARGET_DIR% del /f /s %TARGET_DIR%
mkdir %TARGET_DIR%
mkdir %TARGET_DIR%\plugins
mkdir %TARGET_DIR%\plugins\bin
mkdir %TARGET_DIR%\plugins\imageformats
mkdir %TARGET_DIR%\plugins\sqldrivers
mkdir %TARGET_DIR%\settings
mkdir %TARGET_DIR%\translations
mkdir %TARGET_DIR%\icons

rem === SHARED COMPONENTS ===

rem - Boost -
copy %BOOST_BIN_DIR%\boost_date_time-vc90-mt-1_39.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_filesystem-vc90-mt-1_39.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_system-vc90-mt-1_39.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_thread-vc90-mt-1_39.dll %TARGET_DIR%

rem - Qt -
copy %QT_BIN_DIR%\phonon4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtCore4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtGui4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtNetwork4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtScript4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtSql4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtSvg4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtWebKit4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtXml4.dll %TARGET_DIR%

copy %QT_BIN_DIR%\..\plugins\imageformats\qgif4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qico4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qjpeg4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qmng4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qsvg4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qtiff4.dll %TARGET_DIR%\plugins\imageformats

copy %QT_BIN_DIR%\..\plugins\sqldrivers\qsqlite4.dll %TARGET_DIR%\plugins\sqldrivers

rem - OpenSSL -
copy %OPENSSL_BIN_DIR%\libeay32.dll %TARGET_DIR%
copy %OPENSSL_BIN_DIR%\ssleay32.dll %TARGET_DIR%

rem - libtorrent -
copy %LIBTORRENT_BIN_DIR%\torrent.dll %TARGET_DIR%

rem - libcurl -
copy %LIBCURL_BIN_DIR%\libcurl.dll %TARGET_DIR%

rem === LEECHCRAFT FILES ===

rem - Main files -
copy %LEECHCRAFT_BUILD_DIR%\%BUILD_TYPE%\leechcraft.exe %TARGET_DIR%
copy %LEECHCRAFT_BUILD_DIR%\plugininterface\%BUILD_TYPE%\plugininterface.dll %TARGET_DIR%
copy %LEECHCRAFT_BUILD_DIR%\xmlsettingsdialog\%BUILD_TYPE%\xmlsettingsdialog.dll %TARGET_DIR%

rem - Plugins -
copy %LEECHCRAFT_BUILD_DIR%\plugins\aggregator\%BUILD_TYPE%\leechcraft_aggregator.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\bittorrent\%BUILD_TYPE%\leechcraft_bittorrent.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\chatter\%BUILD_TYPE%\leechcraft_chatter.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\cstp\%BUILD_TYPE%\leechcraft_cstp.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\deadlyrics\%BUILD_TYPE%\leechcraft_deadlyrics.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\historyholder\%BUILD_TYPE%\leechcraft_historyholder.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\lcftp\%BUILD_TYPE%\leechcraft_lcftp.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\networkmonitor\%BUILD_TYPE%\leechcraft_networkmonitor.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\poshuku\%BUILD_TYPE%\leechcraft_poshuku.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\seekthru\%BUILD_TYPE%\leechcraft_seekthru.dll %TARGET_DIR%\plugins\bin
rem copy %LEECHCRAFT_BUILD_DIR%\plugins\yasd\%BUILD_TYPE%\leechcraft_yasd.dll %TARGET_DIR%\plugins\bin

rem - Poshuku sub-plugins -
copy %LEECHCRAFT_BUILD_DIR%\plugins\poshuku\plugins\cleanweb\%BUILD_TYPE%\leechcraft_poshuku_cleanweb.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\poshuku\plugins\filescheme\%BUILD_TYPE%\leechcraft_poshuku_filescheme.dll %TARGET_DIR%\plugins\bin
copy %LEECHCRAFT_BUILD_DIR%\plugins\poshuku\plugins\fua\%BUILD_TYPE%\leechcraft_poshuku_fua.dll %TARGET_DIR%\plugins\bin

rem - Settings -
copy %LEECHCRAFT_ROOT_DIR%\src\coresettings.xml %TARGET_DIR%\settings

copy %LEECHCRAFT_ROOT_DIR%\src\plugins\aggregator\aggregatorsettings.xml %TARGET_DIR%\settings
copy %LEECHCRAFT_ROOT_DIR%\src\plugins\bittorrent\torrentsettings.xml %TARGET_DIR%\settings
copy %LEECHCRAFT_ROOT_DIR%\src\plugins\cstp\cstpsettings.xml %TARGET_DIR%\settings
copy %LEECHCRAFT_ROOT_DIR%\src\plugins\poshuku\poshukusettings.xml %TARGET_DIR%\settings
copy %LEECHCRAFT_ROOT_DIR%\src\plugins\seekthru\seekthrusettings.xml %TARGET_DIR%\settings

copy %LEECHCRAFT_ROOT_DIR%\src\plugins\poshuku\plugins\cleanweb\poshukucleanwebsettings.xml %TARGET_DIR%\settings
copy %LEECHCRAFT_ROOT_DIR%\src\plugins\poshuku\plugins\fua\poshukufuasettings.xml %TARGET_DIR%\settings

rem - Translations -
for /r %LEECHCRAFT_ROOT_DIR%\src %%f in (*.qm) do copy %%f %TARGET_DIR%\translations

rem - Oxygen icon theme -
copy %LEECHCRAFT_ROOT_DIR%\src\iconsets\oxygen\oxygen.mapping %TARGET_DIR%\icons
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\iconsets\oxygen\icons %TARGET_DIR%\icons\oxygen

rem - Other stuff -
copy %LEECHCRAFT_ROOT_DIR%\tools\win32installer\qt.conf %TARGET_DIR%