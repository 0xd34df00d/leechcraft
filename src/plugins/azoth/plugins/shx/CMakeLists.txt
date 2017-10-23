cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_shx)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (SHX_SRCS
	shx.cpp
	xmlsettingsmanager.cpp
	)
CreateTrs ("azoth_shx" "en;ru_RU" SHX_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_shx" "en;ru_RU" "${SHX_SRCS}" "" "azothshxsettings.xml")
set (SHX_RESOURCES
	shxresources.qrc
	)
QtAddResources (SHX_RCCS ${SHX_RESOURCES})

add_library (leechcraft_azoth_shx SHARED
	${SHX_COMPILED_TRANSLATIONS}
	${SHX_SRCS}
	${SHX_RCCS}
	)
target_link_libraries (leechcraft_azoth_shx
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_shx DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothshxsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_shx Core Widgets)
