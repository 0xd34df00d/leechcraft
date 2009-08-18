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
	)
SET (RESOURCES fsirc.qrc)
SET (UIS
	fsmain.ui
	fsircview.ui
	)

QT4_WRAP_UI (C_UIS ${UIS})
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})


ADD_LIBRARY (leechcraft_chatter SHARED
	${SRCS}
	${UIS}
	${MOC_SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_chatter
	${QT_LIBRARIES}
	plugininterface
	)
INSTALL (TARGETS leechcraft_chatter DESTINATION ${LC_PLUGINS_DEST})
