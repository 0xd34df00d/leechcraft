INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (KEYWORDS_SRCS
	keywords.cpp
	keywordsmanagerwidget.cpp
	editkeyworddialog.cpp
	xmlsettingsmanager.cpp
	)
SET (KEYWORDS_HEADERS
	keywords.h
	keywordsmanagerwidget.h
	editkeyworddialog.h
	xmlsettingsmanager.h
	)
SET (KEYWORDS_FORMS
	keywordsmanagerwidget.ui
	editkeyworddialog.ui
	)
SET (KEYWORDS_RESOURCES
	poshukukeywordsresources.qrc
	)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (KEYWORDS_MOC_SRCS ${KEYWORDS_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (KEYWORDS_UIS_H ${KEYWORDS_FORMS})
QT4_ADD_RESOURCES (KEYWORDS_RCCS ${KEYWORDS_RESOURCES})
ADD_LIBRARY (leechcraft_poshuku_keywords SHARED
	${KEYWORDS_SRCS}
	${KEYWORDS_MOC_SRCS}
	${KEYWORDS_UIS_H}
	${KEYWORDS_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_keywords
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poshuku_keywords DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES poshukukeywordssettings.xml DESTINATION ${LC_SETTINGS_DEST})
