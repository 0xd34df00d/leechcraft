CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_xtazy)
INCLUDE (InitLCPlugin OPTIONAL)

IF (NOT WIN32)
	SET (QT_USE_QTDBUS TRUE)
ENDIF (NOT WIN32)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	xtazy.cpp
	xmlsettingsmanager.cpp
	tunesourcebase.cpp
	filesource.cpp
	lcsource.cpp
	)

IF (QT_QTDBUS_FOUND)
	ADD_DEFINITIONS (-DHAVE_DBUS)

	SET (SRCS
		${SRCS}
		mprissource.cpp
		)
ENDIF (QT_QTDBUS_FOUND)

CreateTrs("xtazy" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("xtazy" "en;ru_RU" "${SRCS}" "${FORMS}" "xtazysettings.xml")

ADD_LIBRARY (leechcraft_xtazy SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_xtazy
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_xtazy DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES xtazysettings.xml DESTINATION ${LC_SETTINGS_DEST})
