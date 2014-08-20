include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (KEYWORDS_SRCS
	keywords.cpp
	keywordsmanagerwidget.cpp
	editkeyworddialog.cpp
	xmlsettingsmanager.cpp
	)
set (KEYWORDS_FORMS
	keywordsmanagerwidget.ui
	editkeyworddialog.ui
	)
set (KEYWORDS_RESOURCES
	poshukukeywordsresources.qrc
	)

CreateTrs ("poshuku_keywords" "en;ru_RU" KEYWORDS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_keywords" "en;ru_RU" "${KEYWORDS_SRCS}" "${KEYWORDS_FORMS}" "poshukukeywordssettings.xml")

QtWrapUi (KEYWORDS_UIS_H ${KEYWORDS_FORMS})
QtAddResources (KEYWORDS_RCCS ${KEYWORDS_RESOURCES})
add_library (leechcraft_poshuku_keywords SHARED
	${KEYWORDS_COMPILED_TRANSLATIONS}
	${KEYWORDS_SRCS}
	${KEYWORDS_UIS_H}
	${KEYWORDS_RCCS}
	)
target_link_libraries (leechcraft_poshuku_keywords
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_keywords DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukukeywordssettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_keywords Widgets)
