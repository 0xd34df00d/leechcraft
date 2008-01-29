PROJECT (leechcraft)
SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET (CMAKE_SKIP_BUILD_RPATH FALSE)
SET (CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
SET (QT_USE_QTNETWORK true)
SET (QT_USE_QTXML true)
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindPNG.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindThreads.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_QTGUI_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} .
	plugininterface/)
ADD_SUBDIRECTORY(exceptions)
ADD_SUBDIRECTORY(plugininterface)
ADD_SUBDIRECTORY(settingsdialog)
ADD_SUBDIRECTORY(xmlsettingsdialog)
ADD_SUBDIRECTORY(plugins/http)
ADD_SUBDIRECTORY(plugins/batcher)
ADD_SUBDIRECTORY(plugins/torrent)
ADD_SUBDIRECTORY(plugins/cron)
ADD_SUBDIRECTORY(plugins/updater)
ADD_SUBDIRECTORY(plugins/mailleecher)

SET (SRCS
	mainwindow.cpp
	commonjobadder.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	plugininfo.cpp
	pluginlisttablewidgeticon.cpp
	changelogdialog.cpp
	logshower.cpp
	xmlsettingsmanager.cpp)

SET (HEADERS
	mainwindow.h
	commonjobadder.h
	core.h
	pluginmanager.h
	plugininfo.h
	pluginlisttablewidgeticon.h
	changelogdialog.h
	logshower.h
	xmlsettingsmanager.h
	interfaces/interfaces.h
	interfaces/structures.h)

SET (FORMS changelogdialog.ui commonjobadder.ui)
SET (RESOURCES mainresources.qrc)

ADD_DEFINITIONS(-DQT_GUI_LIB)
ADD_DEFINITIONS(-D_REENTRANT)

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
