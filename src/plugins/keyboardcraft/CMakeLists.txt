CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_keyboardcraft)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

SET (SRCS
	keyboardcraft.cpp
	keyboardlayoutswitcher.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	keyboardcraft.h
	keyboardlayoutswitcher.h
	xmlsettingsmanager.h
	)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

ADD_LIBRARY (leechcraft_keyboardcraft SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_keyboardcraft
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_keyboardcraft DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES keyboardcraftsettings.xml DESTINATION ${LC_SETTINGS_DEST})
