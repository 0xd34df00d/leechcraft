@echo off

call "%~dp0\winvars32.bat"

set BOOST_LIB_SUFFIX=""
set QT_LIB_SUFFIX=""
rem Boost and Qt libraries have Debug suffixies in names
if "%BUILD_TYPE%" == "Debug" (
	set BOOST_LIB_SUFFIX="gd-"
	set QT_LIB_SUFFIX="d"
)

rem This is the directory where LeechCraft will live
set TARGET_DIR="LeechCraft"

rem === DIRECTORY STRUCTURE ===
if exist %TARGET_DIR% rmdir /s /q %TARGET_DIR%
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
copy %BOOST_BIN_DIR%\boost_date_time-vc100-mt-%BOOST_LIB_SUFFIX%%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_filesystem-vc100-mt-%BOOST_LIB_SUFFIX%%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_program_options-vc100-mt-%BOOST_LIB_SUFFIX%%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_system-vc100-mt-%BOOST_LIB_SUFFIX%%BOOST_VERSION%.dll %TARGET_DIR%
copy %BOOST_BIN_DIR%\boost_thread-vc100-mt-%BOOST_LIB_SUFFIX%%BOOST_VERSION%.dll %TARGET_DIR%


rem - Qt -
copy %QT_BIN_DIR%\phonon%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtCore%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtDeclarative%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtGui%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtMultimedia%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtNetwork%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtScript%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtSql%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtSvg%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtWebKit%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtXml%QT_LIB_SUFFIX%4.dll %TARGET_DIR%
copy %QT_BIN_DIR%\QtXmlPatterns%QT_LIB_SUFFIX%4.dll %TARGET_DIR%

copy %QT_BIN_DIR%\..\plugins\imageformats\qgif%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qico%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qjpeg%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qmng%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qsvg%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\imageformats
copy %QT_BIN_DIR%\..\plugins\imageformats\qtiff%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\imageformats

copy %QT_BIN_DIR%\..\plugins\iconengines\qsvgicon%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\iconengines

copy %QT_BIN_DIR%\..\plugins\phonon_backend\phonon_ds9%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\phonon_backend

copy %QT_BIN_DIR%\..\plugins\sqldrivers\qsqlite%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\sqldrivers
copy %QT_BIN_DIR%\..\plugins\sqldrivers\qsqlpsql%QT_LIB_SUFFIX%4.dll %TARGET_DIR%\plugins\sqldrivers

rem copy %QT_BIN_DIR%\..\translations\qt_*.qm %TARGET_DIR%\translations

rem - OpenSSL -
copy %OPENSSL_BIN_DIR%\libeay32.dll %TARGET_DIR%
copy %OPENSSL_BIN_DIR%\ssleay32.dll %TARGET_DIR%

rem - libtorrent -
copy %LIBTORRENT_BIN_DIR%\torrent.dll %TARGET_DIR%

rem - qjson -
copy %QJSON_BIN_DIR%\qjson.dll %TARGET_DIR%

rem - VLC -
copy %VLC_DIR%\libvlc.dll %TARGET_DIR%
copy %VLC_DIR%\libvlccore.dll %TARGET_DIR%

rem === LEECHCRAFT FILES ===

rem - Main files -
copy %LEECHCRAFT_BUILD_DIR%\%BUILD_TYPE%\leechcraft.exe %TARGET_DIR%
copy %LEECHCRAFT_BUILD_DIR%\util\%BUILD_TYPE%\lcutil.dll %TARGET_DIR%
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

rem - Azoth resources -
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\plugins\azoth\share %TARGET_DIR%\share
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\plugins\azoth\plugins\standardstyles\share %TARGET_DIR%\share
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\plugins\azoth\plugins\nativeemoticons\share %TARGET_DIR%\share

rem - Kinotify themes -
mkdir %TARGET_DIR%\share\kinotify\themes
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\plugins\kinotify\themes %TARGET_DIR%\share\kinotify\themes

rem - AdvancedNotifications stuff -
xcopy /e /i %LEECHCRAFT_ROOT_DIR%\src\plugins\advancednotifications\share %TARGET_DIR%\share

rem - Other stuff -
copy %LEECHCRAFT_ROOT_DIR%\tools\win32\installer\qt.conf %TARGET_DIR%

rem === COMPILE QT TRANSLATIONS ===
for %%f in (%QT_BIN_DIR%\..\translations\qt_*.ts) do %QT_BIN_DIR%\lrelease %%f
copy %QT_BIN_DIR%\..\translations\qt_*.qm %TARGET_DIR%\translations

rem == Copy install script to Leechcraft
XCOPY installer\* %TARGET_DIR% /Y

pause