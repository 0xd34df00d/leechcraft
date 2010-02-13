IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_chatter)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${Boost_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	fscmdedit.cpp
	fsettings.cpp
	fsirc.cpp
	fsircview.cpp
	fstrayicon.cpp
	irc.cpp
	ircserver.cpp
	chatter.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	fscmdedit.h
	fsettings.h
	fsirc.h
	fsircview.h
	fstrayicon.h
	ircdefs.h
	irc.h
	ircserver.h
	config.h
	chatter.h
	xmlsettingsmanager.h
	)
SET (RESOURCES fsirc.qrc)
SET (UIS
	fsmain.ui
	fsircview.ui
	)
SET (TRANSLATIONS
	leechcraft_chatter_ru_RU.ts
	leechcraft_chatter_ar.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_chatter_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_chatter_ar.qm
	)

QT4_WRAP_UI (C_UIS ${UIS})
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_chatter SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_chatter
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_chatter DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES chattersettings.xml DESTINATION ${LC_SETTINGS_DEST})
