cmake_minimum_required (VERSION 2.8)
project (leechcraft_kbswitch)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	kbswitch.cpp
	keyboardlayoutswitcher.cpp
	xmlsettingsmanager.cpp
	)
set (HEADERS
	kbswitch.h
	keyboardlayoutswitcher.h
	xmlsettingsmanager.h
	)
set (RESOURCES kbswitchresources.qrc)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_kbswitch SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_kbswitch
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_kbswitch DESTINATION ${LC_PLUGINS_DEST})
install (FILES kbswitchsettings.xml DESTINATION ${LC_SETTINGS_DEST})
