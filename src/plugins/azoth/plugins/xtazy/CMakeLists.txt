INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)
SET (XTAZY_SRCS
	xtazy.cpp
	tunesourcebase.cpp
	filesource.cpp
	xmlsettingsmanager.cpp
	)
SET (XTAZY_HEADERS
	xtazy.h
	tunesourcebase.h
	filesource.h
	xmlsettingsmanager.h
	)

IF (QT_QTDBUS_FOUND)
	ADD_DEFINITIONS (-DHAVE_DBUS)

	SET (XTAZY_SRCS
		${XTAZY_SRCS}
		mprissource.cpp
		)
	SET (XTAZY_HEADERS
		${XTAZY_HEADERS}
		mprissource.h
		)
ENDIF (QT_QTDBUS_FOUND)

QT4_WRAP_CPP (XTAZY_MOC_SRCS ${XTAZY_HEADERS})

ADD_LIBRARY (leechcraft_azoth_xtazy SHARED
	${XTAZY_SRCS}
	${XTAZY_MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_xtazy
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_xtazy DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES azothxtazysettings.xml DESTINATION ${LC_SETTINGS_DEST})