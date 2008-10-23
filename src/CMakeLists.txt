CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
IF (COMMAND cmake_policy)
  cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (leechcraft)
SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
LIST (APPEND CMAKE_MODULE_PATH "cmake")
SET (QT_USE_QTNETWORK true)
SET (QT_USE_QTXML true)
SET (QT_USE_QTWEBKIT true)
SET (QT_USE_QTSQL true)
SET (QT_USE_QTSCRIPT true)
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/GX")
	SET (CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "-fvisibility=hidden -Wall")
ENDIF (UNIX)
IF (WIN32 AND NOT MSVC)
	SET (CMAKE_CXX_FLAGS "-Wall")
ENDIF (WIN32 AND NOT MSVC)
SET (LC_PLUGINS_DEST "lib/leechcraft/plugins")
# Check modules first in local dir
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem system thread)
FIND_PACKAGE (Qt4 REQUIRED)
FIND_PACKAGE (Threads REQUIRED)
FIND_PACKAGE (RBTorrent)

IF (ENABLE_PACK)
	INCLUDE (InstallRequiredSystemLibraries)
	SET (CPACK_CMAKE_GENERATOR "Unix Makefiles")
	SET (CPACK_PACKAGE_NAME "leechcraft")
	SET (CPACK_GENERATOR "DEB;RPM")
	SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Extensible crossplatform download manager")
	SET (CPACK_PACKAGE_VENDOR "0xd34df00d")

	FIND_PROGRAM(DPKG_CMD dpkg)
	IF(NOT DPKG_CMD)
		MESSAGE(STATUS "Can not find dpkg in your path, default to i386.")
		SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
	ENDIF(NOT DPKG_CMD)
	EXECUTE_PROCESS(COMMAND "${DPKG_CMD}" --print-architecture
		OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)

	SET (CPACK_DEBIAN_PACKAGE_SECTION "net")
	SET (CPACK_DEBIAN_PACKAGE_DESCRIPTION "Extensible crossplatform download manager
 LeechCraft is a cross-platform extensibe download manager written
 in C++. Currently it offers full-featured BitTorrent client, feed
 reader, HTTP/FTP plugin, Remote access and much more. It also aims
 to be resource-efficient, working quite well on even old computers.")
	SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libqt4-core (>= 4.4.3), libqt4-gui (>= 4.4.3), libqt4-network (>= 4.4.3), libqt4-webkit (>= 4.4.3), libqt4-xml (>= 4.4.3), libqt4-sql (>= 4.4.3), libqt4-script (>= 4.4.3), libtorrent-rasterbar-0.14")
	SET (CPACK_PACKAGE_DESCRIPTION_FILE	"${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
	SET (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/copyright.txt")
	SET (CPACK_PACKAGE_VERSION_MAJOR "0")
	SET (CPACK_PACKAGE_VERSION_MINOR "2")
	FIND_PACKAGE (Subversion)
	IF (Subversion_FOUND)
		Subversion_WC_INFO (${PROJECT_SOURCE_DIR} ProjectSVNInfo)
		SET (CPACK_PACKAGE_VERSION_PATCH ${ProjectSVNInfo_WC_LAST_CHANGED_REV})
	ELSE (Subversion_FOUND)
		SET (CPACK_PACKAGE_VERSION_PATCH "9999_svn")
	ENDIF (Subversion_FOUND)
	SET (CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
	SET (CPACK_PACKAGE_CONTACT "0xd34df00d@gmail.com")
	SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "Rudoy Georg <0xd34df00d@gmail.com>")
	SET (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/scripts/postinst")

	SET (CPACK_SOURCE_GENERATOR "TGZ;ZIP;TBZ2")
	SET (CPACK_SOURCE_IGNORE_FILES "/.svn/;.log")
	SET (CPACK_PACKAGE_EXECUTABLES "leechcraft" "LeechCraft")
	SET (CPACK_STRIP_FILES TRUE)
	INCLUDE (CPack)
ENDIF (ENABLE_PACK)

INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_QTGUI_INCLUDE_DIR}
                     ${QT_QTCORE_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} .
                     ${Boost_INCLUDE_DIR}
	plugininterface/)
ADD_SUBDIRECTORY (plugininterface)
ADD_SUBDIRECTORY (xmlsettingsdialog)
IF (NOT ENABLE_CONFIGURABLE)
	ADD_SUBDIRECTORY(plugins/cstp)
	ADD_SUBDIRECTORY(plugins/batcher)
	IF(RBTorrent_FOUND)
	   	ADD_SUBDIRECTORY(plugins/torrent)
   	ENDIF(RBTorrent_FOUND)
	ADD_SUBDIRECTORY(plugins/remoter)
	ADD_SUBDIRECTORY(plugins/aggregator)
	ADD_SUBDIRECTORY(plugins/chatter)
ELSE (NOT ENABLE_CONFIGURABLE)
	IF (ENABLE_HTTP)
		ADD_SUBDIRECTORY(plugins/cstp)
	ENDIF (ENABLE_HTTP)
	IF (ENABLE_BATCHER)
		ADD_SUBDIRECTORY(plugins/batcher)
	ENDIF (ENABLE_BATCHER)
	IF (ENABLE_TORRENT)
		ADD_SUBDIRECTORY(plugins/torrent)
	ENDIF (ENABLE_TORRENT)
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
	xmlsettingsmanager.cpp
	mergemodel.cpp
	filtermodel.cpp
	pluginmanagerdialog.cpp
	fancypopupmanager.cpp
	)

SET (HEADERS
	mainwindow.h
	commonjobadder.h
	core.h
	pluginmanager.h
	xmlsettingsmanager.h
	mergemodel.h
	filtermodel.h
	pluginmanagerdialog.h
	fancypopupmanager.h
	interfaces/interfaces.h
	interfaces/structures.h
	)

SET (FORMS
	commonjobadder.ui
	leechcraft.ui
	pluginmanagerdialog.ui
	)
SET (RESOURCES mainresources.qrc)
SET (TRANSLATIONS
	leechcraft_ru_RU.qm
	leechcraft_uk_UA.qm
	)

ADD_DEFINITIONS (-DQT_GUI_LIB)
ADD_DEFINITIONS (-D_REENTRANT)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCC ${RESOURCES})

ADD_CUSTOM_TARGET (core_translations
	COMMAND "lrelease" "leechcraft_ru_RU.ts" "leechcraft_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_EXECUTABLE (leechcraft WIN32 ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCC})
ADD_DEPENDENCIES (leechcraft core_translations)
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
