INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

SET (ONLINEBOOKMARKS_SRCS
	onlinebookmarks.cpp
	xmlsettingsmanager.cpp
	settings.cpp
	core.cpp
	delicious/deliciousbookmarksservice.cpp
	)
SET (ONLINEBOOKMARKS_HEADERS
	onlinebookmarks.h
	xmlsettingsmanager.h
	abstractbookmarksservice.h
	settings.h
	core.h
	delicious/deliciousbookmarksservice.h
	)
SET (ONLINEBOOKMARKS_FORMS 
	settings.ui
	)
SET (ONLINEBOOKMARKS_RESOURCES poshukuonlinebookmarksresources.qrc
	delicious/onlinebookmarksdelicious.qrc
	)

QT4_WRAP_CPP (ONLINEBOOKMARKS_MOC_SRCS ${ONLINEBOOKMARKS_HEADERS})
QT4_WRAP_UI (ONLINEBOOKMARKS_UIS_H ${ONLINEBOOKMARKS_FORMS})
QT4_ADD_RESOURCES (ONLINEBOOKMARKS_RCCS ${ONLINEBOOKMARKS_RESOURCES})

ADD_LIBRARY (leechcraft_poshuku_onlinebookmarks SHARED
	${ONLINEBOOKMARKS_SRCS}
	${ONLINEBOOKMARKS_MOC_SRCS}
	${ONLINEBOOKMARKS_UIS_H}
	${ONLINEBOOKMARKS_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_onlinebookmarks
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_onlinebookmarks DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES poshukuonlinebookmarkssettings.xml DESTINATION ${LC_SETTINGS_DEST})