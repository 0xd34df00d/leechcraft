cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_autopaste)
include (InitLCPlugin OPTIONAL)

option (WITH_AZOTH_AUTOPASTE_JSON "Enable support for JSON-based services in Azoth Autopaste module" ON)

set (QJSON_INCLUDES)
set (QJSON_LIBRARIES)

if (WITH_AZOTH_AUTOPASTE_JSON AND NOT USE_QT5)
	find_package (QJSON REQUIRED)
	set (QJSON_INCLUDES ${QJSON_INCLUDE_DIR})
endif ()

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	${QJSON_INCLUDES}
	)

set (AUTOPASTE_SRCS
	actionsstorage.cpp
	autopaste.cpp
	highlight2str.cpp
	bpasteservice.cpp
	codepadservice.cpp
	hastebinservice.cpp
	pasteorgruservice.cpp
	poundpythonservice.cpp
	spacepasteservicebase.cpp
	pastedialog.cpp
	pasteservicebase.cpp
	pasteservicefactory.cpp
	xmlsettingsmanager.cpp
	)

if (WITH_AZOTH_AUTOPASTE_JSON OR USE_QT5)
	add_definitions (-DWITH_JSON)
endif ()

set (AUTOPASTE_FORMS
	pastedialog.ui
	)
set (AUTOPASTE_RESOURCES azothautopasteresources.qrc)

CreateTrs ("azoth_autopaste" "en;es;ru_RU;uk_UA" AUTOPASTE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_autopaste" "en;ru_RU" "${AUTOPASTE_SRCS}" "${AUTOPASTE_FORMS}" "azothautopastesettings.xml")

QtWrapUi (AUTOPASTE_UIS_H ${AUTOPASTE_FORMS})
QtAddResources (AUTOPASTE_RCCS ${AUTOPASTE_RESOURCES})

add_library (leechcraft_azoth_autopaste SHARED
	${AUTOPASTE_COMPILED_TRANSLATIONS}
	${AUTOPASTE_SRCS}
	${AUTOPASTE_RCCS}
	${AUTOPASTE_UIS_H}
	${QJSON_INCLUDE_DIR}
	)
target_link_libraries (leechcraft_azoth_autopaste
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_autopaste DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothautopastesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_autopaste Network Widgets)
