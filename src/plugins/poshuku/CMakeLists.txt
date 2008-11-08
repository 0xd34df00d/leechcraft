INCLUDE_DIRECTORIES (
	${QT_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR}
	${QT_QTWEBKIT_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)
SET (SRCS
	poshuku.cpp
	xmlsettingsmanager.cpp
	core.cpp
	browserwidget.cpp
	customwebview.cpp
	progresslineedit.cpp
	favoritesmodel.cpp
	addtofavoritesdialog.cpp
	)
SET (HEADERS
	poshuku.h
	xmlsettingsmanager.h
	core.h
	browserwidget.h
	customwebview.h
	progresslineedit.h
	favoritesmodel.h
	addtofavoritesdialog.h
	)
SET (FORMS
	poshuku.ui
	browserwidget.ui
	addtofavoritesdialog.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_poshuku_ru_RU.qm
	leechcraft_poshuku_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_TARGET (poshuku_translations
	COMMAND "lrelease" "leechcraft_poshuku_ru_RU.ts" "leechcraft_poshuku_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_LIBRARY (leechcraft_poshuku SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)

ADD_DEPENDENCIES (leechcraft_poshuku poshuku_translations)

TARGET_LINK_LIBRARIES (leechcraft_poshuku
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)

INSTALL (TARGETS leechcraft_poshuku DESTINATION ${LC_PLUGINS_DEST})
