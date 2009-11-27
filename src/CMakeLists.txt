CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
IF (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (leechcraft)
IF (NOT DISABLE_RPATH_TRICKS)
	SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
	SET (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
	SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
ENDIF (NOT DISABLE_RPATH_TRICKS)
LIST (APPEND CMAKE_MODULE_PATH "cmake")
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/GX /W3")
	SET (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD")
	SET (CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "-fvisibility=hidden")
	IF (EXTRA_WARNINGS)
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	ENDIF (EXTRA_WARNINGS)
	IF (USE_PIPE)
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
	ENDIF (USE_PIPE)
ENDIF (UNIX)
IF (WIN32 AND NOT MSVC)
	SET (CMAKE_CXX_FLAGS "-Wall")
ENDIF (WIN32 AND NOT MSVC)

STRING (COMPARE EQUAL ${CMAKE_SIZEOF_VOID_P} "8" IS64BITS)

IF (IS64BITS AND RESPECTLIB64)
	SET (LIBDIR "lib64")
ELSE (IS64BITS AND RESPECTLIB64)
	SET (LIBDIR "lib")
ENDIF (IS64BITS AND RESPECTLIB64)

SET (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins")
SET (LC_TRANSLATIONS_DEST "share/leechcraft/translations")
SET (LC_SETTINGS_DEST "share/leechcraft/settings")

# Check modules first in local dir
SET (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem system thread signals)
FIND_PACKAGE (Qt4 REQUIRED)
FIND_PACKAGE (Phonon)
FIND_PACKAGE (RBTorrent)
#FIND_PACKAGE (Wt)

ADD_SUBDIRECTORY (plugininterface)
ADD_SUBDIRECTORY (xmlsettingsdialog)
ADD_SUBDIRECTORY (iconsets)

SET (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

OPTION (ENABLE_SUMMARY "Buidl Summary, the plugin that shows a handy summary of the stuff going on" ON)
OPTION (ENABLE_TORRENT "Build BitTorrent, the BitTorrent client" ON)
OPTION (ENABLE_AGGREGATOR "Build Aggregator, the RSS/Atom feed reader" ON)
OPTION (ENABLE_POSHUKU "Build Poshuku, the WWW Browser" ON)
OPTION (ENABLE_NUFELLA "Build Nufella, the Gnutella/Gnutella2 client" OFF)
OPTION (ENABLE_HTTP "Build CSTP, the HTTP implementation" ON)
OPTION (ENABLE_FTP "Build LCFTP, the FTP client" ON)
OPTION (ENABLE_LMP "Build LMP, the LeechCraft Media Previewer" ON)
OPTION (ENABLE_DBUSMANAGER "Build DBusManager, the D-Bus support for the LeechCraft" ON)
OPTION (ENABLE_DEADLYRICS "Build DeadLyrics, the client for the lyricwiki.org site" ON)
OPTION (ENABLE_SEEKTHRU "Build SeekThru, the client for the OpenSearch-aware web sites" ON)
OPTION (ENABLE_NETWORKMONITOR "Build NetworkMonitor, the monitor for HTTP requests" ON)
OPTION (ENABLE_HISTORYHOLDER "Build HistoryHolder, the keeper of the history" ON)
OPTION (ENABLE_CHATTER "Build Chatter, simple IRC client" ON)
OPTION (ENABLE_YASD "Build YASD, Yet Another Site Downloader" ON)
OPTION (ENABLE_DCMINATOR "Build DCminator, the DirectConnect client" ON)
OPTION (ENABLE_REMOTER "Build Remoter, the remote access manager" OFF)
OPTION (ENABLE_NEWLIFE "Build NewLife, the settings importer" ON)
OPTION (ENABLE_ANHERO "Build AnHero, the crash handler" ON)
OPTION (ENABLE_VGRABBER "Build vGrabber, the vkontakte.ru music/video getter" ON)

if (ENABLE_SUMMARY)
	ADD_SUBDIRECTORY (plugins/summary)
ENDIF (ENABLE_SUMMARY)

IF (ENABLE_TORRENT)
	IF (RBTorrent_FOUND)
	   	ADD_SUBDIRECTORY (plugins/bittorrent)
	ELSE (RBTorrent_FOUND)
		MESSAGE (ERROR "Rasterbar libtorrent not found, not building BitTorrent")
   	ENDIF (RBTorrent_FOUND)
ENDIF (ENABLE_TORRENT)

IF (ENABLE_AGGREGATOR)
	ADD_SUBDIRECTORY (plugins/aggregator)
ENDIF (ENABLE_AGGREGATOR)

IF (ENABLE_POSHUKU)
	ADD_SUBDIRECTORY (plugins/poshuku)
ENDIF (ENABLE_POSHUKU)

IF (ENABLE_NUFELLA)
	ADD_SUBDIRECTORY (plugins/nufella)
ENDIF (ENABLE_NUFELLA)

IF (ENABLE_CHATTER)
	ADD_SUBDIRECTORY (plugins/chatter)
ENDIF (ENABLE_CHATTER)

IF (ENABLE_HTTP)
	ADD_SUBDIRECTORY (plugins/cstp)
ENDIF (ENABLE_HTTP)

IF (ENABLE_FTP)
	ADD_SUBDIRECTORY (plugins/lcftp)
ENDIF (ENABLE_FTP)

IF (ENABLE_LMP)
	IF (QT_PHONON_FOUND)
		ADD_SUBDIRECTORY (plugins/lmp)
	ENDIF (QT_PHONON_FOUND)
ENDIF (ENABLE_LMP)

IF (ENABLE_DBUSMANAGER)
	IF (QT_QTDBUS_FOUND AND UNIX)
		ADD_SUBDIRECTORY (plugins/dbusmanager)
	ENDIF (QT_QTDBUS_FOUND AND UNIX)
ENDIF (ENABLE_DBUSMANAGER)

IF (ENABLE_DEADLYRICS)
	ADD_SUBDIRECTORY (plugins/deadlyrics)
ENDIF (ENABLE_DEADLYRICS)

IF (ENABLE_SEEKTHRU)
	ADD_SUBDIRECTORY (plugins/seekthru)
ENDIF (ENABLE_SEEKTHRU)

IF (ENABLE_NETWORKMONITOR)
	ADD_SUBDIRECTORY (plugins/networkmonitor)
ENDIF (ENABLE_NETWORKMONITOR)

IF (ENABLE_HISTORYHOLDER)
	ADD_SUBDIRECTORY (plugins/historyholder)
ENDIF (ENABLE_HISTORYHOLDER)

IF (ENABLE_YASD)
	ADD_SUBDIRECTORY (plugins/yasd)
ENDIF (ENABLE_YASD)

IF (ENABLE_DCMINATOR)
	ADD_SUBDIRECTORY (plugins/dcminator)
ENDIF (ENABLE_DCMINATOR)

IF (ENABLE_REMOTER)
	ADD_SUBDIRECTORY (plugins/remoter)
ENDIF (ENABLE_REMOTER)

IF (ENABLE_NEWLIFE)
	ADD_SUBDIRECTORY (plugins/newlife)
ENDIF (ENABLE_NEWLIFE)

IF (ENABLE_ANHERO)
	ADD_SUBDIRECTORY (plugins/anhero)
ENDIF (ENABLE_ANHERO)

IF (ENABLE_VGRABBER)
	ADD_SUBDIRECTORY (plugins/vgrabber)
ENDIF (ENABLE_VGRABBER)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTSCRIPT TRUE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	.
	plugininterface/)

SET (SRCS
	mainwindow.cpp
	commonjobadder.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	xmlsettingsmanager.cpp
	pluginmanagerdialog.cpp
	fancypopupmanager.cpp
	skinengine.cpp
	childactioneventfilter.cpp
	tabcontainer.cpp
	logtoolbox.cpp
	settingssink.cpp
	authenticationdialog.cpp
	sslerrorsdialog.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	debugmessagehandler.cpp
	application.cpp
	iconchooser.cpp
	handlerchoicedialog.cpp
	tabwidget.cpp
	graphwidget.cpp
	shortcutmanager.cpp
	keysequencer.cpp
	networkdiskcache.cpp
	networkaccessmanager.cpp
	coreproxy.cpp
	appstyler.cpp
	tagsmanager.cpp
	tagsviewer.cpp
	directorywatcher.cpp
	aboutdialog.cpp
	startupwizard.cpp
	localsockethandler.cpp
	toolbarguard.cpp
	clipboardwatcher.cpp
	3dparty/qxttooltip.cpp
	)
SET (HEADERS
	mainwindow.h
	commonjobadder.h
	core.h
	pluginmanager.h
	xmlsettingsmanager.h
	pluginmanagerdialog.h
	fancypopupmanager.h
	skinengine.h
	childactioneventfilter.h
	tabcontainer.h
	logtoolbox.h
	settingssink.h
	authenticationdialog.h
	sslerrorsdialog.h
	storagebackend.h
	sqlstoragebackend.h
	debugmessagehandler.h
	application.h
	iconchooser.h
	handlerchoicedialog.h
	tabwidget.h
	graphwidget.h
	shortcutmanager.h
	keysequencer.h
	networkdiskcache.h
	networkaccessmanager.h
	coreproxy.h
	appstyler.h
	tagsmanager.h
	tagsviewer.h
	directorywatcher.h
	aboutdialog.h
	startupwizard.h
	localsockethandler.h
	toolbarguard.h
	clipboardwatcher.h
	3dparty/qxttooltip.h
	3dparty/qxttooltip_p.h
	interfaces/iinfo.h
	interfaces/iwindow.h
	interfaces/idownload.h
	interfaces/ihavesettings.h
	interfaces/icustomprovider.h
	interfaces/imultitabs.h
	interfaces/iembedtab.h
	interfaces/itaggablejobs.h
	interfaces/ijobholder.h
	interfaces/iimportexport.h
	interfaces/ifinder.h
	interfaces/iwebbrowser.h
	interfaces/ientityhandler.h
	interfaces/ipluginready.h
	interfaces/iplugin2.h
	interfaces/ihaveshortcuts.h
	interfaces/itoolbarembedder.h
	interfaces/istartupwizard.h
	interfaces/ipluginadaptor.h
	interfaces/imenuembedder.h
	interfaces/isummaryrepresentation.h
	interfaces/structures.h
	)
SET (FORMS
	commonjobadder.ui
	leechcraft.ui
	pluginmanagerdialog.ui
	logtoolbox.ui
	settingssink.ui
	authenticationdialog.ui
	sslerrorsdialog.ui
	handlerchoicedialog.ui
	shortcutmanager.ui
	keysequencer.ui
	tagsviewer.ui
	aboutdialog.ui
	)
SET (RESOURCES mainresources.qrc)
SET (TRANSLATIONS
	leechcraft_ru_RU.ts
	leechcraft_de_DE.ts
	leechcraft_uk_UA.ts
	leechcraft_it.ts
	leechcraft_ar.ts
	leechcraft_eo.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_de_DE.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_it.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_eo.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCC ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

IF (NOT DEFINED LEECHCRAFT_VERSION)
	FIND_PROGRAM (GIT_CMD git)
	EXECUTE_PROCESS (COMMAND "${GIT_CMD}" describe
		OUTPUT_VARIABLE LEECHCRAFT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	MESSAGE (STATUS "Detected LeechCraft version: ${LEECHCRAFT_VERSION}")
ENDIF (NOT DEFINED LEECHCRAFT_VERSION)

CONFIGURE_FILE (config.h.in config.h)

IF (UNIX)
	ADD_CUSTOM_COMMAND (OUTPUT
		data/leechcraft.1.gz
		COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/data/compress"
		DEPENDS data/leechcraft.1
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
		)
	ADD_EXECUTABLE (leechcraft WIN32
		config.h
		${COMPILED_TRANSLATIONS}
		${SRCS}
		${MOC_SRCS}
		${UIS_H}
		${RCC}
		data/leechcraft.1.gz
		)
ELSE (UNIX)
	ADD_EXECUTABLE (leechcraft WIN32
		config.h
		${COMPILED_TRANSLATIONS}
		${SRCS}
		${MOC_SRCS}
		${UIS_H}
		${RCC}
		)
ENDIF (UNIX)

TARGET_LINK_LIBRARIES(leechcraft
						${QT_LIBRARIES}
						plugininterface
						xmlsettingsdialog)
INSTALL (TARGETS leechcraft DESTINATION bin)
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})

INSTALL (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor
	PATTERN ".svn" EXCLUDE)
IF (UNIX)
	INSTALL (FILES freedesktop/leechcraft.desktop DESTINATION share/applications)
	INSTALL (FILES data/leechcraft.1.gz DESTINATION share/man/man1)
ENDIF (UNIX)
