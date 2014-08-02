cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_sarin)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (SARIN_SRCS
	sarin.cpp
	toxprotocol.cpp
	)
#CreateTrs ("azoth_sarin" "en;ru_RU" SARIN_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_sarin" "en;ru_RU" "${SARIN_SRCS}" "" "")
#set (SARIN_RESOURCES
#	sarinresources.qrc
#	)
#QT4_ADD_RESOURCES (SARIN_RCCS ${SARIN_RESOURCES})

add_library (leechcraft_azoth_sarin SHARED
	${SARIN_COMPILED_TRANSLATIONS}
	${SARIN_SRCS}
	${SARIN_RCCS}
	)
target_link_libraries (leechcraft_azoth_sarin
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_sarin DESTINATION ${LC_PLUGINS_DEST})
#install (FILES azothsarinsettings.xml DESTINATION ${LC_SETTINGS_DEST})
