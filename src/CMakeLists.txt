CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
IF (COMMAND cmake_policy)
		      cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (leechcraft)
SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET (CMAKE_SKIP_BUILD_RPATH FALSE)
SET (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
LIST(APPEND CMAKE_MODULE_PATH "cmake")
SET (QT_USE_QTNETWORK true)
SET (QT_USE_QTXML true)
SET (QT_USE_QTSVG true)
SET (QT_USE_QTWEBKIT true)
SET (LC_PLUGINS_DEST "lib/leechcraft/plugins")
FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(Qt4 REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

INCLUDE (InstallRequiredSystemLibraries)
SET (CPACK_CMAKE_GENERATOR "Unix Makefiles")
SET (CPACK_PACKAGE_NAME "leechcraft")
SET (CPACK_GENERATOR "DEB;RPM")
SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Extensible crossplatform download manager")
SET (CPACK_PACKAGE_VENDOR "0xd34df00d")
SET (CPACK_DEBIAN_PACKAGE_SECTION "net")
SET (CPACK_DEBIAN_PACKAGE_DESCRIPTION "Extensible crossplatform download manager
 LeechCraft is a cross-platform
 extensibe download manager written
 in C++. Currently it offers full-featured BitTorrent client, feed
 reader, HTTP/FTP plugin, Remote access and much more. It also aims
 to be resource-efficient, working quite well on even old computers.")
SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-core (>= 4.4.0), libqt4-gui (>= 4.4.0), libqt4-network (>= 4.4.0), libqt4-svg (>= 4.4.0), libqt4-webkit (>= 4.4.0), libqt4-xml (>= 4.4.0), libtorrent-rasterbar-0.13")
SET (CPACK_PACKAGE_DESCRIPTION_FILE	"${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")
SET (CPACK_PACKAGE_VERSION_MAJOR "0")
SET (CPACK_PACKAGE_VERSION_MINOR "2")
SET (CPACK_PACKAGE_VERSION_PATCH "634")
SET (CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
SET (CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
SET (CPACK_PACKAGE_CONTACT "0xd34df00d@gmail.com")
SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "0xd34df00d")
INCLUDE (CPack)

INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} .
	plugininterface/)
ADD_SUBDIRECTORY(exceptions)
ADD_SUBDIRECTORY(plugininterface)
ADD_SUBDIRECTORY(settingsdialog)
ADD_SUBDIRECTORY(xmlsettingsdialog)
IF (NOT ENABLE_CONFIGURABLE)
	ADD_SUBDIRECTORY(plugins/http)
	ADD_SUBDIRECTORY(plugins/cstp)
	ADD_SUBDIRECTORY(plugins/batcher)
   	ADD_SUBDIRECTORY(plugins/torrent)
	ADD_SUBDIRECTORY(plugins/cron)
	ADD_SUBDIRECTORY(plugins/mailleecher)
	ADD_SUBDIRECTORY(plugins/remoter)
	ADD_SUBDIRECTORY(plugins/aggregator)
	ADD_SUBDIRECTORY(plugins/chatter)
ELSE (NOT ENABLE_CONFIGURABLE)
	IF (ENABLE_HTTP)
		ADD_SUBDIRECTORY(plugins/http)
	ENDIF (ENABLE_HTTP)
	IF (ENABLE_BATCHER)
		ADD_SUBDIRECTORY(plugins/batcher)
	ENDIF (ENABLE_BATCHER)
	IF (ENABLE_TORRENT)
		ADD_SUBDIRECTORY(plugins/torrent)
	ENDIF (ENABLE_TORRENT)
	IF (ENABLE_CRON)
		ADD_SUBDIRECTORY(plugins/cron)
	ENDIF (ENABLE_CRON)
	IF (ENABLE_MAIL)
		ADD_SUBDIRECTORY(plugins/mailleecher)
	ENDIF (ENABLE_MAIL)
	IF (ENABLE_REMOTER)
		ADD_SUBDIRECTORY(plugins/remoter)
	ENDIF (ENABLE_REMOTER)
	IF (ENABLE_AGGREGATOR)
		ADD_SUBDIRECTORY(plugins/aggregator)
	ENDIF (ENABLE_AGGREGATOR)
	IF (ENABLE_CHATTER)
		ADD_SUBDIRECTORY(plugins/chatter)
	ENDIF (ENABLE_CHATTER)
ENDIF (NOT ENABLE_CONFIGURABLE)

SET (SRCS
	mainwindow.cpp
	commonjobadder.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	plugininfo.cpp
	pluginlisttablewidgeticon.cpp
	changelogdialog.cpp
	xmlsettingsmanager.cpp)

SET (HEADERS
	mainwindow.h
	commonjobadder.h
	core.h
	pluginmanager.h
	plugininfo.h
	pluginlisttablewidgeticon.h
	changelogdialog.h
	xmlsettingsmanager.h
	interfaces/interfaces.h
	interfaces/structures.h)

SET (FORMS changelogdialog.ui commonjobadder.ui)
SET (RESOURCES mainresources.qrc)

ADD_DEFINITIONS (-DQT_GUI_LIB)
ADD_DEFINITIONS (-D_REENTRANT)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCC ${RESOURCES})

ADD_EXECUTABLE (leechcraft WIN32 ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCC})
TARGET_LINK_LIBRARIES(leechcraft
						${QT_LIBRARIES}
						plugininterface
						exceptions
						xmlsettingsdialog)
INSTALL (TARGETS leechcraft DESTINATION bin)
