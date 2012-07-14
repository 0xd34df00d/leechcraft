INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FATAPE_SRCS
	fatape.cpp
    userscript.cpp
    greasemonkey.cpp
    xmlsettingsmanager.cpp
    userscriptsmanagerwidget.cpp
    userscriptinstallerdialog.cpp
    resourcedownloadhandler.cpp
	)
SET (FATAPE_HEADERS
	fatape.h
    userscript.h
    greasemonkey.h
    xmlsettingsmanager.h
    userscriptsmanagerwidget.h
    userscriptinstallerdialog.h
    resourcedownloadhandler.h
	)
SET (FATAPE_FORMS
    userscriptsmanagerwidget.ui
    userscriptinstallerdialog.ui
    )
SET (FATAPE_RESOURCES
	poshukufataperesources.qrc
	)
CreateTrVars ("poshuku_fatape" "en;it;ru_RU;uk_UA" FATAPE_TRANSLATIONS FATAPE_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (FATAPE_MOC_SRCS ${FATAPE_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (FATAPE_UIS_H ${FATAPE_FORMS})
QT4_ADD_RESOURCES (FATAPE_RCCS ${FATAPE_RESOURCES})
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
	${FATAPE_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_fatape
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poshuku_fatape DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES poshukufatapesettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (FILES ${FATAPE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
