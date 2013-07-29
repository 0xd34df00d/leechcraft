cmake_minimum_required (VERSION 2.8)
project (leechcraft_kbswitch)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	kbswitch.cpp
	keyboardlayoutswitcher.cpp
	xmlsettingsmanager.cpp
	kbctl.cpp
	quarkproxy.cpp
	flagiconprovider.cpp
	)

set (RESOURCES kbswitchresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_kbswitch SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_kbswitch
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	xkbfile
	)
install (TARGETS leechcraft_kbswitch DESTINATION ${LC_PLUGINS_DEST})
install (FILES kbswitchsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml/kbswitch DESTINATION ${LC_QML_DEST})
