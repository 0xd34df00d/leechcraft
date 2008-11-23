INCLUDE_DIRECTORIES (
	${QT_INCLUDE_DIR}
   	${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
   	${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${Wt_INCLUDE_DIR}
	)

SET (SRCS
    remoter.cpp
    core.cpp
	qtowabstractitemmodeladaptor.cpp
	downloaderswidget.cpp
	qtowtoolbaradaptor.cpp
	util.cpp
	interface.cpp
	)
SET (HEADERS
    remoter.h
    core.h
	qtowabstractitemmodeladaptor.h
	downloaderswidget.h
	qtowtoolbaradaptor.h
	util.h
	interface.h
	)
SET (FORMS
    mainwindow.ui)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_remoter_ru_RU.qm
	)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_NO_KEYWORDS)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_TARGET (remoter_translations
	COMMAND "lrelease" "leechcraft_remoter_ru_RU.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_LIBRARY (leechcraft_remoter SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS})
ADD_DEPENDENCIES (leechcraft_remoter remoter_translations)
TARGET_LINK_LIBRARIES (leechcraft_remoter
	${QT_LIBRARIES}
	${Wt_LIBRARY}
	${Wt_HTTP_LIBRARY}
	${Wt_EXT_LIBRARY}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_remoter DESTINATION ${LC_PLUGINS_DEST})
