cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_herbicide)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (HERBICIDE_SRCS
	herbicide.cpp
	confwidget.cpp
	xmlsettingsmanager.cpp
	)
set (HERBICIDE_FORMS
	confwidget.ui
	)
set (HERBICIDE_RESOURCES azothherbicideresources.qrc)

CreateTrs ("azoth_herbicide" "en;es;ru_RU;uk_UA" HERBICIDE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_herbicide" "en;ru_RU" "${HERBICIDE_SRCS}" "${HERBICIDE_FORMS}" "azothherbicidesettings.xml")

QT4_WRAP_UI (HERBICIDE_UIS_H ${HERBICIDE_FORMS})
QT4_ADD_RESOURCES (HERBICIDE_RCCS ${HERBICIDE_RESOURCES})

add_library (leechcraft_azoth_herbicide SHARED
	${HERBICIDE_COMPILED_TRANSLATIONS}
	${HERBICIDE_SRCS}
	${HERBICIDE_UIS_H}
	${HERBICIDE_RCCS}
	)
target_link_libraries (leechcraft_azoth_herbicide
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_herbicide RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${HERBICIDE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES azothherbicidesettings.xml DESTINATION ${LC_SETTINGS_DEST})
