CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
IF (COMMAND cmake_policy)
cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)

PROJECT (leechcraft_woodpecker)
INCLUDE (InitLCPlugin OPTIONAL)

IF (NOT CMAKE_MODULE_PATH)
SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
ENDIF (NOT CMAKE_MODULE_PATH)

FIND_LIBRARY (kqoauth /usr/lib64)
FIND_LIBRARY (qjson /usr/lib64)
FIND_PACKAGE (LeechCraft REQUIRED)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QSCINTILLA_INCLUDE_DIR}
	${QT_INCLUDE_DIR}
	)
SET (SRCS
	core.cpp
	tweet.cpp
	twitdelegate.cpp
	twitterinterface.cpp
	twitterpage.cpp
	twitteruser.cpp
	woodpecker.cpp
	xmlsettingsmanager.cpp
	)
SET (RESOURCES
	woodpeckerresources.qrc
	)
SET (FORMS
	twitterpage.ui
	)
# CreateTrs ("woodpecker" "en;ru_RU" COMPILED_TRANSLATIONS)
# CreateTrsUpTarget("woodpecker" "en;ru_RU" "${SRCS}" "${FORMS}" "woodpeckersettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_woodpecker SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_woodpecker
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	qjson
	kqoauth
	)
	
INSTALL (TARGETS leechcraft_woodpecker DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES woodpeckersettings.xml DESTINATION ${LC_SETTINGS_DEST})
