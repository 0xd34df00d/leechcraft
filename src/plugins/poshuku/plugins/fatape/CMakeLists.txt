INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FATAPE_SRCS
	fatape.cpp
    userscript.cpp
    greasemonkey.cpp
    xmlsettingsmanager.cpp
    userscriptsmanagerwidget.cpp
    userscriptinstallerwidget.cpp
    resourcedownloadhandler.cpp
	)
SET (FATAPE_HEADERS
	fatape.h
    userscript.h
    greasemonkey.h
    xmlsettingsmanager.h
    userscriptsmanagerwidget.h
    userscriptinstallerwidget.h
    resourcedownloadhandler.h
	)
SET (FATAPE_FORMS
    userscriptsmanagerwidget.ui
    userscriptinstallerwidget.ui
    )
CreateTrVars ("poshuku_fatape" "en;ru_RU;uk_UA" FATAPE_TRANSLATIONS FATAPE_COMPILED_TRANSLATIONS)

QT4_WRAP_CPP (FATAPE_MOC_SRCS ${FATAPE_HEADERS})
QT4_WRAP_UI (FATAPE_UIS_H ${FATAPE_FORMS})
ADD_CUSTOM_COMMAND (OUTPUT
	${FATAPE_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${FATAPE_TRANSLATIONS}
	DEPENDS ${FATAPE_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_poshuku_fatape SHARED
	${FATAPE_SRCS}
	${FATAPE_MOC_SRCS}
    ${FATAPE_UIS_H}
	${FATAPE_COMPILED_TRANSLATIONS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_fatape
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_fatape DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES poshukufatapesettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${FATAPE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
