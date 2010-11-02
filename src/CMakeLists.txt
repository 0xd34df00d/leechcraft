cmake_minimum_required(VERSION 2.6)
IF (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (leechcraft)
#IF (NOT DISABLE_RPATH_TRICKS)
#	SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#	SET (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#	SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
#ENDIF (NOT DISABLE_RPATH_TRICKS)
LIST (APPEND CMAKE_MODULE_PATH "cmake")
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/GX /W3")
	SET (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD")
	SET (CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fno-strict-aliasing")
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

IF (GPTOOLS_CPU AND GPTOOLS_MEM)
	SET (GPTOOLS_CPU FALSE)
	SET (GPTOOLS_MEM FALSE)
	SET (GPTOOLS_CPU_AND_MEM TRUE)
ENDIF (GPTOOLS_CPU AND GPTOOLS_MEM)
IF (GPTOOLS_MEM)
	MESSAGE (STATUS "Enabled memory profiling via Google perftools")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc")
ENDIF (GPTOOLS_MEM)
IF (GPTOOLS_CPU)
	MESSAGE (STATUS "Enabled CPU profiling via Google perftools")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
ENDIF (GPTOOLS_CPU)
IF (GPTOOLS_CPU_AND_MEM)
	MESSAGE (STATUS "Enabled both CPU and memory profiling via Google perftools")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc_and_profiler")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc_and_profiler")
ENDIF (GPTOOLS_CPU_AND_MEM)

STRING (COMPARE EQUAL ${CMAKE_SIZEOF_VOID_P} "8" IS64BITS)

IF (IS64BITS AND RESPECTLIB64)
	SET (LIBDIR "lib64")
ELSE (IS64BITS AND RESPECTLIB64)
	SET (LIBDIR "lib")
ENDIF (IS64BITS AND RESPECTLIB64)

SET (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins")
SET (LC_TRANSLATIONS_DEST "share/leechcraft/translations")
SET (LC_SETTINGS_DEST "share/leechcraft/settings")
SET (LC_INSTALLEDMANIFEST_DEST "share/leechcraft/installed")

# Check modules first in local dir
SET (CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem program_options system thread)
FIND_PACKAGE (Qt4 REQUIRED)
#FIND_PACKAGE (Wt)

ADD_SUBDIRECTORY (plugininterface)
ADD_SUBDIRECTORY (xmlsettingsdialog)
ADD_SUBDIRECTORY (iconsets)

SET (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
SET (LEECHCRAFT_LIBRARIES plugininterface xmlsettingsdialog)

OPTION (ENABLE_SUMMARY "Build Summary, the plugin that shows a handy summary of the stuff going on" ON)
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
OPTION (ENABLE_YASD "Build YASD, Yet Another Site Downloader" OFF)
OPTION (ENABLE_REMOTER "Build Remoter, the remote access manager" OFF)
OPTION (ENABLE_NEWLIFE "Build NewLife, the settings importer" ON)
OPTION (ENABLE_ANHERO "Build AnHero, the crash handler" ON)
OPTION (ENABLE_VGRABBER "Build vGrabber, the vkontakte.ru music/video getter" ON)
OPTION (ENABLE_POC "Build PoC, text editor" OFF)
OPTION (ENABLE_KINOTIFY "Build Kinotify, fancy kinetic notifications" ON)
OPTION (ENABLE_AUSCRIE "Build Auscrie, the auto screenshooter" ON)
OPTION (ENABLE_TABPP "Build Tab++, the enhanced tab manager" ON)
OPTION (ENABLE_EISKALTDCPP "Build EiskaltDC++ client" OFF)
OPTION (ENABLE_AZOTH "Build Azoth, the IM client" OFF)
OPTION (ENABLE_SECMAN "Build SecMan, security & personal data manager" OFF)
OPTION (ENABLE_QROSP "Build Qrosp, the scripting support plugin" OFF)
OPTION (ENABLE_POPISHU "Build Qrosp, the scripting support plugin" OFF)
OPTION (ENABLE_LACKMAN "Build LackMan, the package manager for LeechCraft" OFF)
OPTION (ENABLE_SYNCER "Build Syncer, the synchronization plugin for LeechCraft" OFF)
OPTION (ENABLE_SHELLOPEN "Build ShellOpen, the plugin supporting handling entities via external applications" ON)
ENABLE_TESTING ()

IF (ENABLE_SUMMARY)
	ADD_SUBDIRECTORY (plugins/summary)
ENDIF (ENABLE_SUMMARY)

IF (ENABLE_TORRENT)
	ADD_SUBDIRECTORY (plugins/bittorrent)
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
	ADD_SUBDIRECTORY (plugins/lmp)
ENDIF (ENABLE_LMP)

IF (ENABLE_DBUSMANAGER)
	ADD_SUBDIRECTORY (plugins/dbusmanager)
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

IF (ENABLE_POC)
	ADD_SUBDIRECTORY (plugins/poc)
ENDIF (ENABLE_POC)

IF (ENABLE_KINOTIFY)
	ADD_SUBDIRECTORY (plugins/kinotify)
ENDIF (ENABLE_KINOTIFY)

IF (ENABLE_AUSCRIE)
	ADD_SUBDIRECTORY (plugins/auscrie)
ENDIF (ENABLE_AUSCRIE)

IF (ENABLE_TABPP)
	ADD_SUBDIRECTORY (plugins/tabpp)
ENDIF (ENABLE_TABPP)

IF (ENABLE_EISKALTDCPP)
	ADD_SUBDIRECTORY (plugins/eiskaltdcpp)
ENDIF (ENABLE_EISKALTDCPP)

IF (ENABLE_AZOTH)
	ADD_SUBDIRECTORY (plugins/azoth)
ENDIF (ENABLE_AZOTH)

IF (ENABLE_SECMAN)
	ADD_SUBDIRECTORY (plugins/secman)
ENDIF (ENABLE_SECMAN)

IF (ENABLE_QROSP)
	ADD_SUBDIRECTORY (plugins/qrosp)
ENDIF (ENABLE_QROSP)

IF (ENABLE_POPISHU)
	ADD_SUBDIRECTORY (plugins/popishu)
ENDIF (ENABLE_POPISHU)

IF (ENABLE_LACKMAN)
	ADD_SUBDIRECTORY (plugins/lackman)
ENDIF (ENABLE_LACKMAN)

IF (ENABLE_SYNCER)
	ADD_SUBDIRECTORY (plugins/syncer)
ENDIF (ENABLE_SYNCER)

IF (ENABLE_SHELLOPEN)
	ADD_SUBDIRECTORY (plugins/shellopen)
ENDIF (ENABLE_SHELLOPEN)

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
	tabmanager.cpp
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
	glanceshower.cpp
	glanceitem.cpp
	tabbar.cpp
	wizardtypechoicepage.cpp
	3dparty/qxttooltip.cpp
	fullscreenspacerlabel.cpp
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
	tabmanager.h
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
	glanceshower.h
	glanceitem.h
	tabbar.h
	fullscreenspacerlabel.h
	wizardtypechoicepage.h
	3dparty/qxttooltip.h
	3dparty/qxttooltip_p.h
	interfaces/iinfo.h
	interfaces/iwindow.h
	interfaces/idownload.h
	interfaces/ihavesettings.h
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
	wizardtypechoicepage.ui
	)
SET (RESOURCES mainresources.qrc)
SET (TRANSLATIONS
	leechcraft_ru_RU.ts
	leechcraft_de.ts
	leechcraft_uk_UA.ts
	leechcraft_it.ts
	leechcraft_ar.ts
	leechcraft_eo.ts
	leechcraft_es.ts
	leechcraft_tr.ts
	leechcraft_en.ts
	leechcraft_be.ts
	leechcraft_fr.ts
	leechcraft_pl.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_de.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_it.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_eo.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_es.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_tr.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_en.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_be.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_fr.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_pl.qm
	)
    
ADD_DEFINITIONS (-DBOOST_ALL_NO_LIB)

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
	IF (NOT DEFINED LEECHCRAFT_VERSION)
		IF (WIN32)
			EXECUTE_PROCESS(COMMAND "date" "/T" OUTPUT_VARIABLE
				${LEECHCRAFT_VERSION})
			STRING(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
				${LEECHCRAFT_VERSION} ${${LEECHCRAFT_VERSION}})
		ELSEIF(UNIX)
			EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE
				${LEECHCRAFT_VERSION})
			STRING(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
				${LEECHCRAFT_VERSION} ${${LEECHCRAFT_VERSION}})
		ELSE (WIN32)
			MESSAGE(SEND_ERROR "date not implemented")
			SET(${RESULT} 000000)
		ENDIF (WIN32)
	ENDIF (NOT DEFINED LEECHCRAFT_VERSION)
	MESSAGE (STATUS "Detected LeechCraft version: ${LEECHCRAFT_VERSION}")
ENDIF (NOT DEFINED LEECHCRAFT_VERSION)

CONFIGURE_FILE (config.h.in config.h)
CONFIGURE_FILE (FindLeechCraft.cmake.in FindLeechCraft.cmake @ONLY)

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
						${Boost_PROGRAM_OPTIONS_LIBRARY}
						plugininterface
						xmlsettingsdialog)
INSTALL (TARGETS leechcraft DESTINATION bin)
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})

INSTALL (DIRECTORY plugininterface/ DESTINATION include/leechcraft/plugininterface/
	PATTERN ".h")
INSTALL (DIRECTORY interfaces/ DESTINATION include/leechcraft/interfaces/
	PATTERN ".h")
INSTALL (FILES xmlsettingsdialog/xmlsettingsdialog.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES xmlsettingsdialog/basesettingsmanager.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES xmlsettingsdialog/xsdconfig.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES xmlsettingsdialog/datasourceroles.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/leechcraft/)

INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/FindLeechCraft.cmake DESTINATION share/leechcraft/cmake/)

INSTALL (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor
	PATTERN ".svn" EXCLUDE)
IF (UNIX)
	INSTALL (FILES freedesktop/leechcraft.desktop DESTINATION share/applications)
	INSTALL (FILES data/leechcraft.1.gz DESTINATION share/man/man1)
ENDIF (UNIX)
