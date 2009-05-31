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
	SET (CMAKE_CXX_FLAGS "/GX")
	SET (CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "-fvisibility=hidden -Wall -Wextra")
ENDIF (UNIX)
IF (WIN32 AND NOT MSVC)
	SET (CMAKE_CXX_FLAGS "-Wall")
ENDIF (WIN32 AND NOT MSVC)

IF (IS64BITS AND RESPECTLIB64)
	SET (LIBDIR "lib64")
ELSE (IS64BITS AND RESPECTLIB64)
	SET (LIBDIR "lib")
ENDIF (IS64BITS AND RESPECTLIB64)

SET (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins")
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

SET (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

IF (NOT ENABLE_CONFIGURABLE)
	IF (RBTorrent_FOUND)
	   	ADD_SUBDIRECTORY (plugins/bittorrent)
	ELSE (RBTorrent_FOUND)
		MESSAGE (STATUS "Rasterbar libtorrent not found, not building BitTorrent")
   	ENDIF (RBTorrent_FOUND)
	ADD_SUBDIRECTORY (plugins/aggregator)
	ADD_SUBDIRECTORY (plugins/poshuku)
#	ADD_SUBDIRECTORY (plugins/nufella)
	ADD_SUBDIRECTORY (plugins/cstp)
#	IF (Wt_FOUND)
#		ADD_SUBDIRECTORY (plugins/remoter)
#	ELSE (Wt_FOUND)
#		MESSAGE (STATUS "Wt isn't found, not building Remoter")
#	ENDIF (Wt_FOUND)
	IF (QT_PHONON_FOUND AND NOT WIN32)
		ADD_SUBDIRECTORY (plugins/lmp)
	ENDIF (QT_PHONON_FOUND AND NOT WIN32)
	IF (QT_QTDBUS_FOUND)
		ADD_SUBDIRECTORY (plugins/dbusmanager)
	ENDIF (QT_QTDBUS_FOUND)
	ADD_SUBDIRECTORY (plugins/deadlyrics)
	ADD_SUBDIRECTORY (plugins/seekthru)
	ADD_SUBDIRECTORY (plugins/networkmonitor)
	ADD_SUBDIRECTORY (plugins/historyholder)
ELSE (NOT ENABLE_CONFIGURABLE)
	IF (ENABLE_HTTP)
		ADD_SUBDIRECTORY (plugins/cstp)
	ENDIF (ENABLE_HTTP)
	IF (ENABLE_TORRENT)
		ADD_SUBDIRECTORY (plugins/bittorrent)
	ENDIF (ENABLE_TORRENT)
#	IF (ENABLE_REMOTER)
#		ADD_SUBDIRECTORY (plugins/remoter)
#	ENDIF (ENABLE_REMOTER)
	IF (ENABLE_AGGREGATOR)
		ADD_SUBDIRECTORY (plugins/aggregator)
	ENDIF (ENABLE_AGGREGATOR)
	IF (ENABLE_POSHUKU)
		ADD_SUBDIRECTORY (plugins/poshuku)
	ENDIF (ENABLE_POSHUKU)
	IF (ENABLE_NUFELLA)
		ADD_SUBDIRECTORY (plugins/nufella)
	ENDIF (ENABLE_NUFELLA)
	IF (ENABLE_DBUSMANAGER)
		ADD_SUBDIRECTORY (plugins/dbusmanager)
	ENDIF (ENABLE_DBUSMANAGER)
	IF (ENABLE_LMP)
		ADD_SUBDIRECTORY (plugins/lmp)
	ENDIF (ENABLE_LMP)
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
ENDIF (NOT ENABLE_CONFIGURABLE)

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
	filtermodel.cpp
	pluginmanagerdialog.cpp
	fancypopupmanager.cpp
	skinengine.cpp
	childactioneventfilter.cpp
	tabcontainer.cpp
	zombitechstyle.cpp
	logtoolbox.cpp
	settingssink.cpp
	authenticationdialog.cpp
	sslerrorsdialog.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	debugmessagehandler.cpp
	application.cpp
	iconchooser.cpp
	requestparser.cpp
	handlerchoicedialog.cpp
	categorymerger.cpp
	requestnormalizer.cpp
	operationalmodel.cpp
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
	3dparty/qxttooltip.cpp
	)
SET (HEADERS
	mainwindow.h
	commonjobadder.h
	core.h
	pluginmanager.h
	xmlsettingsmanager.h
	filtermodel.h
	pluginmanagerdialog.h
	fancypopupmanager.h
	skinengine.h
	childactioneventfilter.h
	tabcontainer.h
	zombitechstyle.h
	logtoolbox.h
	settingssink.h
	authenticationdialog.h
	sslerrorsdialog.h
	storagebackend.h
	sqlstoragebackend.h
	debugmessagehandler.h
	application.h
	iconchooser.h
	requestparser.h
	handlerchoicedialog.h
	categorymerger.h
	requestnormalizer.h
	operationalmodel.h
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
	)
SET (RESOURCES mainresources.qrc)
SET (TRANSLATIONS
	leechcraft_ru_RU.ts
	leechcraft_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_uk_UA.qm
	)

ADD_DEFINITIONS (-DQT_GUI_LIB)
ADD_DEFINITIONS (-D_REENTRANT)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCC ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_EXECUTABLE (leechcraft WIN32
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCC})
TARGET_LINK_LIBRARIES(leechcraft
						${QT_LIBRARIES}
						plugininterface
						xmlsettingsdialog)
INSTALL (TARGETS leechcraft DESTINATION bin)

INSTALL (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor
	PATTERN ".svn" EXCLUDE)
IF (UNIX)
	INSTALL (FILES freedesktop/leechcraft.desktop DESTINATION share/applications)
ENDIF (UNIX)
