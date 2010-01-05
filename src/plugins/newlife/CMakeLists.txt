SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	newlife.cpp
	importwizard.cpp
	akregatorimporter.cpp
	akregatorimportpage.cpp
	firstpage.cpp
	lifereaimportpage.cpp
	lifereaimporter.cpp
	ktorrentimportpage.cpp
	ktorrentimporter.cpp
	)
SET (HEADERS
	newlife.h
	importwizard.h
	akregatorimporter.h
	akregatorimportpage.h
	firstpage.h
	lifereaimportpage.h
	lifereaimporter.h
	ktorrentimportpage.h
	ktorrentimporter.h
	abstractimporter.h
	)
SET (FORMS
	importwizard.ui
	feedssettingsimportpage.ui
	firstpage.ui
	)
SET (TRANSLATIONS
	leechcraft_newlife_ru_RU.ts
	)
SET (RESOURCES resources.qrc)
SET (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_newlife_ru_RU.qm
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
ADD_LIBRARY (leechcraft_newlife SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_newlife
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_newlife DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
