cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_tracolor)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (TRACOLOR_SRCS
	tracolor.cpp
	)
#CreateTrs ("azoth_tracolor" "en;ru_RU" TRACOLOR_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_tracolor" "en;ru_RU" "${TRACOLOR_SRCS}" "" "")
set (TRACOLOR_RESOURCES
#	tracolorresources.qrc
	)
QtAddResources (TRACOLOR_RCCS ${TRACOLOR_RESOURCES})

add_library (leechcraft_azoth_tracolor SHARED
	${TRACOLOR_COMPILED_TRANSLATIONS}
	${TRACOLOR_SRCS}
	${TRACOLOR_RCCS}
	)
target_link_libraries (leechcraft_azoth_tracolor
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_tracolor DESTINATION ${LC_PLUGINS_DEST})
#install (FILES azothtracolorsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_tracolor Core)

