cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_autopaste)
include (InitLCPlugin OPTIONAL)

option (WITH_AZOTH_AUTOPASTE_JSON "Enable support for JSON-based services in Azoth Autopaste module" ON)

if (WITH_AZOTH_AUTOPASTE_JSON)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (QJSON REQUIRED)
endif ()

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (AUTOPASTE_SRCS
	autopaste.cpp
	bpasteservice.cpp
	codepadservice.cpp
	hastebinservice.cpp
	pasteorgruservice.cpp
	pastedialog.cpp
	pasteservicebase.cpp
	pasteservicefactory.cpp
	xmlsettingsmanager.cpp
	)
set (AUTOPASTE_HEADERS
	autopaste.h
	bpasteservice.h
	codepadservice.h
	hastebinservice.h
	pasteorgruservice.h
	pastedialog.h
	pasteservicebase.h
	pasteservicefactory.h
	xmlsettingsmanager.h
	)

if (WITH_AZOTH_AUTOPASTE_JSON)
	add_definitions (-DWITH_JSON)

	set (AUTOPASTE_SRCS ${AUTOPASTE_SRCS} hastebinservice.cpp)
endif (WITH_AZOTH_AUTOPASTE_JSON)

set (AUTOPASTE_FORMS
	pastedialog.ui
	)
set (AUTOPASTE_RESOURCES azothautopasteresources.qrc)
CreateTrs ("azoth_autopaste" "en;es;ru_RU;uk_UA" AUTOPASTE_COMPILED_TRANSLATIONS)
QT4_WRAP_UI (AUTOPASTE_UIS_H ${AUTOPASTE_FORMS})
QT4_ADD_RESOURCES (AUTOPASTE_RCCS ${AUTOPASTE_RESOURCES})

add_library (leechcraft_azoth_autopaste SHARED
	${AUTOPASTE_COMPILED_TRANSLATIONS}
	${AUTOPASTE_SRCS}
	${AUTOPASTE_RCCS}
	${AUTOPASTE_UIS_H}
	${QJSON_INCLUDE_DIR}
	)
target_link_libraries (leechcraft_azoth_autopaste
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_autopaste DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothautopastesettings.xml DESTINATION ${LC_SETTINGS_DEST})
