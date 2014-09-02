cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_murm)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT USE_QT5)
	find_package (QJSON REQUIRED)
endif ()

include_directories (${AZOTH_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (MURM_SRCS
	murm.cpp
	vkprotocol.cpp
	vkaccount.cpp
	vkconnection.cpp
	vkentry.cpp
	vkmessage.cpp
	photostorage.cpp
	vcarddialog.cpp
	georesolver.cpp
	xmlsettingsmanager.cpp
	longpollmanager.cpp
	groupsmanager.cpp
	mucjoinwidget.cpp
	entrybase.cpp
	vkchatentry.cpp
	logger.cpp
	accountconfigdialog.cpp
	serverhistorymanager.cpp
	)
set (MURM_FORMS
	vcarddialog.ui
	mucjoinwidget.ui
	accountconfigdialog.ui
	)
set (MURM_RESOURCES azothmurmresources.qrc)

CreateTrs ("azoth_murm" "en;ru_RU" MURM_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_murm" "en;ru_RU" "${MURM_SRCS}" "${MURM_FORMS}" "azothmurmsettings.xml")

QtWrapUi (MURM_UIS_H ${MURM_FORMS})
QtAddResources (MURM_RCCS ${MURM_RESOURCES})

add_library (leechcraft_azoth_murm SHARED
	${MURM_COMPILED_TRANSLATIONS}
	${MURM_SRCS}
	${MURM_UIS_H}
	${MURM_RCCS}
	)
target_link_libraries (leechcraft_azoth_murm
	${QJSON_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_murm DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothmurmsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_murm Network Widgets Xml)
