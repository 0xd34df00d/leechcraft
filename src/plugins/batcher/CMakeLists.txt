INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
    batcher.cpp
	parser.cpp)
SET (HEADERS
    batcher.h
	parser.h)
SET (FORMS
    mainwindow.ui)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_batcher_ru.qm
	leechcraft_batcher_uk.qm
	)

ADD_CUSTOM_COMMAND (OUTPUT ${TRANSLATIONS}
	COMMAND "lrelease" ARGS "leechcraft_batcher_ru.ts" "leechcraft_batcher_uk.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
ADD_LIBRARY (leechcraft_batcher SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS} ${TRANSLATIONS})
TARGET_LINK_LIBRARIES (leechcraft_batcher
                        ${QT_LIBRARIES}
                        plugininterface
                        exceptions
                        settingsdialog)
INSTALL (TARGETS leechcraft_batcher DESTINATION ${LC_PLUGINS_DEST})
