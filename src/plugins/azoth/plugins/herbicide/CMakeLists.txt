cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_herbicide)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (HERBICIDE_SRCS
	herbicide.cpp
	confwidget.cpp
	xmlsettingsmanager.cpp
	logger.cpp
	listsholder.cpp
	)
set (HERBICIDE_FORMS
	confwidget.ui
	)
set (HERBICIDE_RESOURCES azothherbicideresources.qrc)

CreateTrs ("azoth_herbicide" "en;es;ru_RU;uk_UA" HERBICIDE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_herbicide" "en;ru_RU" "${HERBICIDE_SRCS}" "${HERBICIDE_FORMS}" "azothherbicidesettings.xml")

QtWrapUi (HERBICIDE_UIS_H ${HERBICIDE_FORMS})
QtAddResources (HERBICIDE_RCCS ${HERBICIDE_RESOURCES})

add_library (leechcraft_azoth_herbicide SHARED
	${HERBICIDE_COMPILED_TRANSLATIONS}
	${HERBICIDE_SRCS}
	${HERBICIDE_UIS_H}
	${HERBICIDE_RCCS}
	)
target_link_libraries (leechcraft_azoth_herbicide
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_herbicide DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothherbicidesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_herbicide Sql Widgets)
