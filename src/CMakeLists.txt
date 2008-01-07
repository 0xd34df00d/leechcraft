PROJECT (leechcraft)
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindPNG.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/FindThreads.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
ADD_SUBDIRECTORY(plugins/http)
ADD_SUBDIRECTORY(plugins/batcher)
ADD_SUBDIRECTORY(plugins/torrent)
ADD_SUBDIRECTORY(plugins/cron)
ADD_SUBDIRECTORY(plugins/updater)

SET (SRCS
	mainwindow.cpp
	core.cpp
	main.cpp
	pluginmanager.cpp
	plugininfo.cpp
	pluginlisttablewidgeticon.cpp
	changelogdialog.cpp
	logshower.cpp)

SET (HEADERS
	mainwindow.h
	core.h
	pluginmanager.h
	plugininfo.h
	pluginlisttablewidgeticon.h
	changelogdialog.h
	logshower.h
	interfaces/interfaces.h
	interfaces/structures.h)

SET (FORMS changelogdialog.ui)
SET (RESOURCES mainresources.qrc)

ADD_DEFINITIONS(-DQT_GUI_LIB)
ADD_DEFINITIONS(-D_REENTRANT)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCC ${RESOURCES})

ADD_EXECUTABLE (leechcraft ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCC})
TARGET_LINK_LIBRARIES(leechcraft
						${QT_LIBRARIES}
						plugininterface
						exceptions
						settingsdialog)
INSTALL (TARGETS leechcraft DESTINATION bin)
