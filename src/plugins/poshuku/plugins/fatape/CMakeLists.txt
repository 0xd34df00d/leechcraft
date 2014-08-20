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
set (FATAPE_FORMS
	userscriptsmanagerwidget.ui
	userscriptinstallerdialog.ui
	)
set (FATAPE_RESOURCES
	poshukufataperesources.qrc
	)
CreateTrs ("poshuku_fatape" "en;it;ru_RU;uk_UA" FATAPE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_fatape" "en;ru_RU" "${FATAPE_SRCS}" "${FATAPE_FORMS}" "poshukufatapesettings.xml")

QtWrapUi (FATAPE_UIS_H ${FATAPE_FORMS})
QtAddResources (FATAPE_RCCS ${FATAPE_RESOURCES})
add_library (leechcraft_poshuku_fatape SHARED
	${FATAPE_SRCS}
    ${FATAPE_UIS_H}
	${FATAPE_COMPILED_TRANSLATIONS}
	${FATAPE_RCCS}
	)
target_link_libraries (leechcraft_poshuku_fatape
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_fatape DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukufatapesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_fatape Widgets)
