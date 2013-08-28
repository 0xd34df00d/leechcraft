include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (KEYWORDS_SRCS
	keywords.cpp
	keywordsmanagerwidget.cpp
	editkeyworddialog.cpp
	xmlsettingsmanager.cpp
	)
set (KEYWORDS_HEADERS
	keywords.h
	keywordsmanagerwidget.h
	editkeyworddialog.h
	xmlsettingsmanager.h
	)
set (KEYWORDS_FORMS
	keywordsmanagerwidget.ui
	editkeyworddialog.ui
	)
set (KEYWORDS_RESOURCES
	poshukukeywordsresources.qrc
	)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (KEYWORDS_MOC_SRCS ${KEYWORDS_HEADERS})
endif ()
QT4_WRAP_UI (KEYWORDS_UIS_H ${KEYWORDS_FORMS})
QT4_ADD_RESOURCES (KEYWORDS_RCCS ${KEYWORDS_RESOURCES})
add_library (leechcraft_poshuku_keywords SHARED
	${KEYWORDS_SRCS}
	${KEYWORDS_MOC_SRCS}
	${KEYWORDS_UIS_H}
	${KEYWORDS_RCCS}
	)
target_link_libraries (leechcraft_poshuku_keywords
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_keywords RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukukeywordssettings.xml DESTINATION ${LC_SETTINGS_DEST})
