INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
	${QT_QTNETWORK_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR} ../.. .)

SET (SRCS
	cstp.cpp
	core.cpp
	task.cpp
	hook.cpp
	addtask.cpp
	xmlsettingsmanager.cpp
	mainviewdelegate.cpp
	historymodel.cpp
	representationmodel.cpp
	)
SET (HEADERS
	cstp.h
	core.h
	task.h
	hook.h
	addtask.h
	xmlsettingsmanager.h
	mainviewdelegate.h
	historymodel.h
	representationmodel.h
	../../xmlsettingsdialog/basesettingsmanager.h
	)
SET (FORMS
	addtask.ui
	tabwidget.ui
	)
SET (RESOURCES
	resources.qrc
	)
SET (TRANSLATIONS
	leechcraft_cstp_ru_RU.qm
	leechcraft_cstp_uk_UA.qm
	)
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_TARGET (cstp_translations
	COMMAND "lrelease" "leechcraft_cstp_ru_RU.ts" "leechcraft_cstp_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_LIBRARY (leechcraft_cstp SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS})
ADD_DEPENDENCIES (leechcraft_cstp cstp_translations)
LINK_DIRECTORIES (${Boost_LIBRARY_DIR})
TARGET_LINK_LIBRARIES (leechcraft_cstp
                        ${QT_LIBRARIES}
                        plugininterface
                        xmlsettingsdialog)
INSTALL (TARGETS leechcraft_cstp DESTINATION ${LC_PLUGINS_DEST})
