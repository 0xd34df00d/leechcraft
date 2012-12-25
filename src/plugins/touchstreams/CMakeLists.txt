CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_touchstreams)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	touchstreams.cpp
	authmanager.cpp
	audiosearch.cpp
	xmlsettingsmanager.cpp
	)

CreateTrs ("touchstreams" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("touchstreams" "en;ru_RU" "${SRCS}" "${FORMS}" "touchstreamssettings.xml")

ADD_LIBRARY (leechcraft_touchstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_touchstreams
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_touchstreams DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES touchstreamssettings.xml DESTINATION ${LC_SETTINGS_DEST})
