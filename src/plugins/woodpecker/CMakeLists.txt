IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_woodpecker)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	find_library (kqoauth /usr/lib64)
	find_library (qjson /usr/lib64)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

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
	twitterinterface.cpp
	twitterpage.cpp
	twitteruser.cpp
	woodpecker.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	core.h
	tweet.h
	twitterinterface.h
	twitterpage.h
	twitteruser.h
	woodpecker.h
	xmlsettingsmanager.h
	)
SET (RESOURCES
	woodpeckerresources.qrc
	)
SET (FORMS
	twitterpage.ui
	)
#CreateTrVars ("woodpecker" "de;en;fr;it;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	 QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

#ADD_CUSTOM_COMMAND (OUTPUT
#	${COMPILED_TRANSLATIONS}
#	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
#	DEPENDS ${TRANSLATIONS}
#	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#	)
ADD_LIBRARY (leechcraft_woodpecker SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${HEADERS}
	${MOC_SRCS}
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
