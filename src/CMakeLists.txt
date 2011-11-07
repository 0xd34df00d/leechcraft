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
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/EHa /W3")
	SET (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD /D NDEBUG")
	SET (CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /GT /MDd")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fno-strict-aliasing")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
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

IF (NOT LIBDIR)
	SET (LIBDIR "lib${LIB_SUFFIX}")
ENDIF (NOT LIBDIR)

SET (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins")
SET (LC_TRANSLATIONS_DEST "share/leechcraft/translations")
SET (LC_SETTINGS_DEST "share/leechcraft/settings")
SET (LC_QML_DEST "share/leechcraft/qml")
SET (LC_INSTALLEDMANIFEST_DEST "share/leechcraft/installed")
SET (LC_GLOBAL_SETTINGS_DEST "share/leechcraft/rc")
SET (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")

SET (Boost_ADDITIONAL_VERSIONS "1.47" "1.46" "1.46.1" "1.44" "1.44.0")
FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem program_options system thread)
FIND_PACKAGE (Qt4 REQUIRED)

STRING (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr" STANDARD_PREFIX)
IF (NOT STANDARD_PREFIX)
	STRING (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr/local" STANDARD_PREFIX)
ENDIF ()

IF (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)
	ADD_DEFINITIONS (-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
ENDIF (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)

ADD_SUBDIRECTORY (util)
ADD_SUBDIRECTORY (xmlsettingsdialog)
ADD_SUBDIRECTORY (iconsets)

SET (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
SET (LEECHCRAFT_LIBRARIES lcutil xmlsettingsdialog)

# Basic & traditional set of options
OPTION (ENABLE_AGGREGATOR "Build Aggregator, the RSS/Atom feed reader" ON)
OPTION (ENABLE_ANHERO "Build AnHero, the crash handler" ON)
OPTION (ENABLE_AUSCRIE "Build Auscrie, the auto screenshooter" ON)
OPTION (ENABLE_DBUSMANAGER "Build DBusManager, the D-Bus support for the LeechCraft" ON)
OPTION (ENABLE_DEADLYRICS "Build DeadLyrics, the client for the lyricwiki.org site" ON)
OPTION (ENABLE_HISTORYHOLDER "Build HistoryHolder, the keeper of the history" ON)
OPTION (ENABLE_HTTP "Build CSTP, the HTTP implementation" ON)
OPTION (ENABLE_KINOTIFY "Build Kinotify, fancy kinetic notifications" ON)
OPTION (ENABLE_LMP "Build LMP, the LeechCraft Media Previewer" ON)
OPTION (ENABLE_NETWORKMONITOR "Build NetworkMonitor, the monitor for HTTP requests" ON)
OPTION (ENABLE_NEWLIFE "Build NewLife, the settings importer" ON)
OPTION (ENABLE_POSHUKU "Build Poshuku, the WWW Browser" ON)
OPTION (ENABLE_SEEKTHRU "Build SeekThru, the client for the OpenSearch-aware web sites" ON)
OPTION (ENABLE_SUMMARY "Build Summary, the plugin that shows a handy summary of the stuff going on" ON)
OPTION (ENABLE_TABPP "Build Tab++, the enhanced tab manager" ON)
OPTION (ENABLE_TORRENT "Build BitTorrent, the BitTorrent client" ON)
OPTION (ENABLE_VGRABBER "Build vGrabber, the vkontakte.ru music/video getter" ON)

# New
OPTION (ENABLE_ADVANCEDNOTIFICATIONS "Build Advanced Notifications module for more customizable notifications" OFF)
OPTION (ENABLE_AZOTH "Build Azoth, the IM client" OFF)
OPTION (ENABLE_BLACKDASH "Build BlackDash, the dashboard plugin for LeechCraft" OFF)
OPTION (ENABLE_CHOROID "Build Choroid, image viewer" OFF)
OPTION (ENABLE_EISKALTDCPP "Build EiskaltDC++ client" OFF)
OPTION (ENABLE_GLANCE "Quick overview of tabs" OFF)
OPTION (ENABLE_GMAILNOTIFIER "Build GMail Notifier which notifies about new mail in your GMail inbox" OFF)
OPTION (ENABLE_KNOWHOW "Build KnowHow for displaying tips of the day" OFF)
OPTION (ENABLE_LACKMAN "Build LackMan, the package manager for LeechCraft" OFF)
OPTION (ENABLE_POPISHU "Build Popishu, a simple QScintilla-based text editor" OFF)
OPTION (ENABLE_QROSP "Build Qrosp, the scripting support plugin" OFF)
OPTION (ENABLE_SECMAN "Build SecMan, security & personal data manager" OFF)
OPTION (ENABLE_SHELLOPEN "Build ShellOpen, the plugin supporting handling entities via external applications" ON)
OPTION (ENABLE_SYNCER "Build Syncer, the synchronization plugin for LeechCraft" OFF)
OPTION (ENABLE_TABSLIST "Quick overview of tabs" OFF)
OPTION (ENABLE_VFSCORE "Build VFS core" OFF)
OPTION (ENABLE_SNAILS "Build Snails, LeechCraft mail client" OFF)

# Defunct
OPTION (ENABLE_FTP "Build LCFTP, the FTP client" ON)
OPTION (ENABLE_JUFFED "Build JuffEd, the advanced text editor" OFF)
OPTION (ENABLE_OTZERKALU "Build Otzerkalu, which allows one to download web sites recursively" OFF)
OPTION (ENABLE_LAURE "Build Laure, the new generation audio/video player" OFF)
OPTION (ENABLE_TRANSPARENTARCH "Build TransparentArch, the plugin for transparent access to archives in various formats" OFF)

ENABLE_TESTING ()

OPTION (ENABLE_RECOMMENDED "Build all new recommended plugins" OFF)

IF (ENABLE_RECOMMENDED)
	SET (ENABLE_ADVANCEDNOTIFICATIONS TRUE)
	SET (ENABLE_AZOTH TRUE)
	SET (ENABLE_BLACKDASH TRUE)
	SET (ENABLE_EISKALTDCPP TRUE)
	SET (ENABLE_GLANCE TRUE)
	SET (ENABLE_GMAILNOTIFIER TRUE)
	SET (ENABLE_LACKMAN TRUE)
	SET (ENABLE_POPISHU TRUE)
	SET (ENABLE_QROSP TRUE)
	SET (ENABLE_SECMAN TRUE)
	SET (ENABLE_SHELLOPEN TRUE)
	SET (ENABLE_SYNCER TRUE)
	SET (ENABLE_TABSLIST TRUE)
	SET (ENABLE_VFSCORE TRUE)
	SET (ENABLE_KNOWHOW TRUE)
ENDIF (ENABLE_RECOMMENDED)

FUNCTION (CreateTrVars PlugName Langs TranVar CompiledTranVar)
	STRING (LENGTH "${PlugName}" _pnameLength)
	SET (_basename "leechcraft")
	IF (_pnameLength)
		SET (_basename "${_basename}_${PlugName}")
	ENDIF (_pnameLength)
	FOREACH (_LANG ${Langs})
		SET (_tran ${_tran} ${_basename}_${_LANG}.ts)
		SET (_ctran ${_ctran} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}_${_LANG}.qm)
	ENDFOREACH (_LANG)
	SET (${TranVar} ${_tran} PARENT_SCOPE)
	SET (${CompiledTranVar} ${_ctran} PARENT_SCOPE)
ENDFUNCTION (CreateTrVars)

IF (ENABLE_SUMMARY)
	ADD_SUBDIRECTORY (plugins/summary)
ENDIF (ENABLE_SUMMARY)

IF (ENABLE_TORRENT)
	ADD_SUBDIRECTORY (plugins/bittorrent)
ENDIF (ENABLE_TORRENT)

IF (ENABLE_AGGREGATOR)
	ADD_SUBDIRECTORY (plugins/aggregator)
ENDIF (ENABLE_AGGREGATOR)

IF (ENABLE_CHOROID)
	ADD_SUBDIRECTORY (plugins/choroid)
ENDIF (ENABLE_CHOROID)

IF (ENABLE_POSHUKU)
	ADD_SUBDIRECTORY (plugins/poshuku)
ENDIF (ENABLE_POSHUKU)

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

IF (ENABLE_NEWLIFE)
	ADD_SUBDIRECTORY (plugins/newlife)
ENDIF (ENABLE_NEWLIFE)

IF (ENABLE_ANHERO)
	ADD_SUBDIRECTORY (plugins/anhero)
ENDIF (ENABLE_ANHERO)

IF (ENABLE_VGRABBER)
	ADD_SUBDIRECTORY (plugins/vgrabber)
ENDIF (ENABLE_VGRABBER)

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

IF (ENABLE_LAURE)
	ADD_SUBDIRECTORY (plugins/laure)
ENDIF (ENABLE_LAURE)

IF (ENABLE_TRANSPARENTARCH)
	ADD_SUBDIRECTORY (plugins/transparentarch)
ENDIF (ENABLE_TRANSPARENTARCH)

IF (ENABLE_JUFFED)
	ADD_SUBDIRECTORY (plugins/juffed)
ENDIF (ENABLE_JUFFED)

IF (ENABLE_GMAILNOTIFIER)
	ADD_SUBDIRECTORY (plugins/gmailnotifier)
ENDIF (ENABLE_GMAILNOTIFIER)

IF (ENABLE_OTZERKALU)
	ADD_SUBDIRECTORY (plugins/otzerkalu)
ENDIF (ENABLE_OTZERKALU)

IF (ENABLE_ADVANCEDNOTIFICATIONS)
	ADD_SUBDIRECTORY (plugins/advancednotifications)
ENDIF (ENABLE_ADVANCEDNOTIFICATIONS)

IF (ENABLE_GLANCE)
	ADD_SUBDIRECTORY (plugins/glance)
ENDIF (ENABLE_GLANCE)

IF (ENABLE_TABSLIST)
	ADD_SUBDIRECTORY (plugins/tabslist)
ENDIF (ENABLE_TABSLIST)

IF (ENABLE_BLACKDASH)
	ADD_SUBDIRECTORY (plugins/blackdash)
ENDIF (ENABLE_BLACKDASH)

IF (ENABLE_VFSCORE)
	ADD_SUBDIRECTORY (plugins/vfscore)
ENDIF (ENABLE_VFSCORE)

IF (ENABLE_KNOWHOW)
	ADD_SUBDIRECTORY (plugins/knowhow)
ENDIF (ENABLE_KNOWHOW)

IF (ENABLE_SNAILS)
	ADD_SUBDIRECTORY (plugins/snails)
ENDIF (ENABLE_SNAILS)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTSCRIPT TRUE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR})

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
	authenticationdialog.cpp
	sslerrorsdialog.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	debugmessagehandler.cpp
	application.cpp
	handlerchoicedialog.cpp
	graphwidget.cpp
	shortcutmanager.cpp
	keysequencer.cpp
	networkdiskcache.cpp
	networkaccessmanager.cpp
	coreproxy.cpp
	tagsmanager.cpp
	tagsviewer.cpp
	directorywatcher.cpp
	aboutdialog.cpp
	startupwizard.cpp
	localsockethandler.cpp
	toolbarguard.cpp
	clipboardwatcher.cpp
	wizardtypechoicepage.cpp
	3dparty/qxttooltip.cpp
	newtabmenumanager.cpp
	plugintreebuilder.cpp
	coreinstanceobject.cpp
	settingstab.cpp
	separatetabbar.cpp
	separatetabwidget.cpp
	mwproxy.cpp
	winwarndialog.cpp
	coreplugin2manager.cpp
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
	authenticationdialog.h
	sslerrorsdialog.h
	storagebackend.h
	sqlstoragebackend.h
	debugmessagehandler.h
	application.h
	handlerchoicedialog.h
	graphwidget.h
	shortcutmanager.h
	keysequencer.h
	networkdiskcache.h
	networkaccessmanager.h
	coreproxy.h
	tagsmanager.h
	tagsviewer.h
	directorywatcher.h
	aboutdialog.h
	startupwizard.h
	localsockethandler.h
	toolbarguard.h
	clipboardwatcher.h
	newtabmenumanager.h
	wizardtypechoicepage.h
	plugintreebuilder.h
	coreinstanceobject.h
	settingstab.h
	separatetabbar.h
	separatetabwidget.h
	mwproxy.h
	winwarndialog.h
	coreplugin2manager.h
	3dparty/qxttooltip.h
	3dparty/qxttooltip_p.h
	interfaces/structures.h
	)
SET (FORMS
	commonjobadder.ui
	leechcraft.ui
	pluginmanagerdialog.ui
	logtoolbox.ui
	authenticationdialog.ui
	sslerrorsdialog.ui
	handlerchoicedialog.ui
	shortcutmanager.ui
	keysequencer.ui
	tagsviewer.ui
	aboutdialog.ui
	wizardtypechoicepage.ui
	settingstab.ui
	)
SET (RESOURCES mainresources.qrc)

SOURCE_GROUP ("core\\headers" FILES ${HEADERS})
SOURCE_GROUP ("core\\sources" FILES ${SRCS})
SOURCE_GROUP ("core\\forms" FILES ${FORMS})
SOURCE_GROUP ("core\\resources" FILES ${RESOURCES})

CreateTrVars ("" "ru_RU;de;uk_UA;it;ar;eo;es;tr;en;be;fr;pl" TRANSLATIONS COMPILED_TRANSLATIONS)

ADD_DEFINITIONS (-DBOOST_PROGRAM_OPTIONS_DYN_LINK)
ADD_DEFINITIONS (${QT_DEFINITIONS})

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCC ${RESOURCES})

SOURCE_GROUP ("core\\generated_qt" FILES ${MOC_SRCS} ${UIS_H} ${RCC})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

SOURCE_GROUP("core\\translations" FILES ${COMPILED_TRANSLATIONS})

IF (NOT DEFINED LEECHCRAFT_VERSION)
	FIND_PROGRAM (GIT_CMD git)
	EXECUTE_PROCESS (COMMAND "${GIT_CMD}" "--git-dir=${CMAKE_CURRENT_SOURCE_DIR}/../.git" describe
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

TARGET_LINK_LIBRARIES (leechcraft
		${QT_QTMAIN_LIBRARY}
		${QT_LIBRARIES}
		${Boost_PROGRAM_OPTIONS_LIBRARY}
		${LEECHCRAFT_LIBRARIES}
		)
INSTALL (TARGETS leechcraft DESTINATION bin)
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES coresettings.xml DESTINATION ${LC_SETTINGS_DEST})

INSTALL (DIRECTORY util/ DESTINATION include/leechcraft/util/
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
	INSTALL (FILES scripts/leechcraft-add-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
ENDIF (UNIX)
