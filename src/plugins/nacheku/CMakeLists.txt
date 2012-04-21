CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_nacheku)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	nacheku.cpp
	xmlsettingsmanager.cpp
	clipboardwatcher.cpp
	directorywatcher.cpp
	)
SET (HEADERS
	nacheku.h
	xmlsettingsmanager.h
	clipboardwatcher.h
	directorywatcher.h
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

ADD_LIBRARY (leechcraft_nacheku SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_nacheku
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_nacheku DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES nachekusettings.xml DESTINATION ${LC_SETTINGS_DEST})
