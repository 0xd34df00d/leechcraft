CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_sb2)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (QJSON REQUIRED)

SET (QT_USE_QTDECLARATIVE TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)
SET (SRCS
	sb2.cpp
	sbview.cpp
	viewmanager.cpp
	desaturateeffect.cpp
	quarkproxy.cpp
	quarkmanager.cpp
	quarksettingsmanager.cpp
	tablistview.cpp
	tabunhidelistview.cpp
	launcherdroparea.cpp
	launchercomponent.cpp
	lcmenucomponent.cpp
	traycomponent.cpp
	baseactioncomponent.cpp
	dockactioncomponent.cpp
	quarkunhidelistview.cpp
	quarkorderview.cpp
	declarativewindow.cpp
	viewgeometrymanager.cpp
	)

SET (SB2_RESOURCES sb2resources.qrc)
QT4_ADD_RESOURCES (SB2_RCCS ${SB2_RESOURCES})

ADD_LIBRARY (leechcraft_sb2 SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${SB2_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_sb2
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_sb2 DESTINATION ${LC_PLUGINS_DEST})
INSTALL (DIRECTORY share/qml/sb2 DESTINATION ${LC_QML_DEST})
