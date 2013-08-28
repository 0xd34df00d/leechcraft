include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FATAPE_SRCS
	fatape.cpp
    userscript.cpp
    greasemonkey.cpp
    xmlsettingsmanager.cpp
    userscriptsmanagerwidget.cpp
    userscriptinstallerdialog.cpp
    resourcedownloadhandler.cpp
	)
set (FATAPE_HEADERS
	fatape.h
    userscript.h
    greasemonkey.h
    xmlsettingsmanager.h
    userscriptsmanagerwidget.h
    userscriptinstallerdialog.h
    resourcedownloadhandler.h
	)
set (FATAPE_FORMS
    userscriptsmanagerwidget.ui
    userscriptinstallerdialog.ui
    )
set (FATAPE_RESOURCES
	poshukufataperesources.qrc
	)
CreateTrs ("poshuku_fatape" "en;it;ru_RU;uk_UA" FATAPE_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (FATAPE_MOC_SRCS ${FATAPE_HEADERS})
endif ()
QT4_WRAP_UI (FATAPE_UIS_H ${FATAPE_FORMS})
QT4_ADD_RESOURCES (FATAPE_RCCS ${FATAPE_RESOURCES})
add_library (leechcraft_poshuku_fatape SHARED
	${FATAPE_SRCS}
	${FATAPE_MOC_SRCS}
    ${FATAPE_UIS_H}
	${FATAPE_COMPILED_TRANSLATIONS}
	${FATAPE_RCCS}
	)
target_link_libraries (leechcraft_poshuku_fatape
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_fatape DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukufatapesettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${FATAPE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
