FIND_PACKAGE (CURL)
IF (NOT CURL_FOUND)
	MESSAGE (ERROR "libcurl not found")
ENDIF (NOT CURL_FOUND)

SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	lcftp.cpp
	core.cpp
	worker.cpp
	inactiveworkersfilter.cpp
	xmlsettingsmanager.cpp
	structures.cpp
	watchthread.cpp
	pane.cpp
	tabwidget.cpp
	tabmanager.cpp
	3dparty/ftpparse.c
	)
SET (HEADERS
	lcftp.h
	core.h
	worker.h
	inactiveworkersfilter.h
	xmlsettingsmanager.h
	structures.h
	watchthread.h
	pane.h
	tabwidget.h
	tabmanager.h
	3dparty/ftpparse.h
	)
SET (FORMS
	pane.ui
	tabwidget.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_lcftp_ru_RU.ts
	leechcraft_lcftp_uk_UA.ts
	)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lcftp_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_lcftp_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_lcftp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_lcftp
	${QT_LIBRARIES}
	${CURL_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_lcftp DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES lcftpsettings.xml DESTINATION ${LC_SETTINGS_DEST})
