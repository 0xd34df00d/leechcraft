SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
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
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_chatter_ru_RU.qm
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
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_chatter DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES chattersettings.xml DESTINATION ${LC_SETTINGS_DEST})
