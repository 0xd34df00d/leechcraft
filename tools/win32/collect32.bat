rem @echo off

rem Set these variables to proper paths of your system:
set BOOST_BIN_DIR="C:\Boost\1.42\stage\lib"
set BOOST_VERSION="1_42"
set LIBTORRENT_BIN_DIR="C:/Programming/libtorrent/bin/msvc-9.0/release/boost-link-shared/boost-source/threading-multi/"
set LIBCURL_BIN_DIR="C:/Programming/curl/curl-7.20.0/lib/Release/"
set OPENSSL_BIN_DIR="C:/Programming/openssl/openssl-0.9.8m/out32dll/"
set QT_BIN_DIR="c:\Qt\4.6.2\bin"

set LEECHCRAFT_ROOT_DIR="..\.."
set LEECHCRAFT_BUILD_DIR="build32"
set BUILD_TYPE="MinSizeRel"

rem This is the directory where LeechCraft will live
set TARGET_DIR="LeechCraft"

rem === DIRECTORY STRUCTURE ===
if exist %TARGET_DIR% del /f /s /q %TARGET_DIR%
mkdir %TARGET_DIR%
mkdir %TARGET_DIR%\plugins
mkdir %TARGET_DIR%\plugins\bin
mkdir %TARGET_DIR%\plugins\imageformats
mkdir %TARGET_DIR%\plugins\sqldrivers
mkdir %TARGET_DIR%\plugins\phonon_backend
mkdir %TARGET_DIR%\plugins\iconengines
mkdir %TARGET_DIR%\settings
mkdir %TARGET_DIR%\translations
mkdir %TARGET_DIR%\leechcraft
mkdir %TARGET_DIR%\leechcraft\themes
mkdir %TARGET_DIR%\icons

rem === SHARED COMPONENTS ===

rem - Boost -
copy %BOOST_BIN_DIR%\boost_date_time-vc90-mt-%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_filesystem-vc90-mt-%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_system-vc90-mt-%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_thread-vc90-mt-%BOOST_VERSION%.dll %TARGET_DIR%

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
copy %QT_BIN_DIR%\QtXmlPatterns4.dll %TARGET_DIR%

copy %QT_BIN_DIR%\..\plugins\imageformats\qgif4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qico4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qjpeg4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qmng4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qsvg4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qtiff4.dll %TARGET_DIR%\plugins\imageformats

copy %QT_BIN_DIR%\..\plugins\iconengines\qsvgicon4.dll %TARGET_DIR%\plugins\iconengines

copy %QT_BIN_DIR%\..\plugins\phonon_backend\phonon_ds94.dll %TARGET_DIR%\plugins\phonon_backend

copy %QT_BIN_DIR%\..\plugins\sqldrivers\qsqlite4.dll %TARGET_DIR%\plugins\sqldrivers
copy %QT_BIN_DIR%\..\plugins\sqldrivers\qsqlpsql4.dll %TARGET_DIR%\plugins\sqldrivers

rem copy %QT_BIN_DIR%\..\translations\qt_*.qm %TARGET_DIR%\translations

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
for /r %LEECHCRAFT_BUILD_DIR%\plugins %%f in (%BUILD_TYPE%\leechcraft_*.dll) do copy %%f %TARGET_DIR%\plugins\bin

rem - Settings -
for /r %LEECHCRAFT_ROOT_DIR%\src %%f in (*settings.xml) do copy %%f %TARGET_DIR%\settings

rem - Translations -
for /r %LEECHCRAFT_ROOT_DIR%\src %%f in (*.qm) do copy %%f %TARGET_DIR%\translations

rem - Oxygen icon theme -
copy %LEECHCRAFT_ROOT_DIR%\src\iconsets\oxygen\oxygen.mapping %TARGET_DIR%\icons
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\iconsets\oxygen\icons %TARGET_DIR%\icons\oxygen
copy nul %TARGET_DIR%\leechcraft\themes\oxygen

rem - Other stuff -
copy %LEECHCRAFT_ROOT_DIR%\tools\win32\installer\qt.conf %TARGET_DIR%
pause