INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FATAPE_SRCS
	fatape.cpp
    userscript.cpp
    greasemonkey.cpp
    xmlsettingsmanager.cpp
    userscriptsmanager.cpp
	)
SET (FATAPE_HEADERS
	fatape.h
    userscript.h
    greasemonkey.h
    xmlsettingsmanager.h
    userscriptsmanager.h
	)
SET (FATAPE_FORMS
    userscripts.ui
    )

QT4_WRAP_CPP (FATAPE_MOC_SRCS ${FATAPE_HEADERS})
QT4_WRAP_UI (FATAPE_UIS_H ${FATAPE_FORMS})
ADD_LIBRARY (leechcraft_poshuku_fatape SHARED
	${FATAPE_SRCS}
	${FATAPE_MOC_SRCS}
    ${FATAPE_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_fatape
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_fatape DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES poshukufatapesettings.xml DESTINATION ${LC_SETTINGS_DEST})
