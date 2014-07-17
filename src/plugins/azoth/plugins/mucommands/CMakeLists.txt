cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_mucommands)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (MUCOMMANDS_SRCS
	mucommands.cpp
	commands.cpp
	descparser.cpp
	)

set (MUCOMMANDS_RESOURCES azothmucommandsresources.qrc)
QT4_ADD_RESOURCES (MUCOMMANDS_RCCS ${MUCOMMANDS_RESOURCES})

CreateTrs ("azoth_mucommands" "en;ru_RU" MUCOMMANDS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_mucommands" "en;ru_RU" "${MUCOMMANDS_SRCS}" "" "")

add_library (leechcraft_azoth_mucommands SHARED
	${MUCOMMANDS_COMPILED_TRANSLATIONS}
	${MUCOMMANDS_RCCS}
	${MUCOMMANDS_SRCS}
	)
target_link_libraries (leechcraft_azoth_mucommands
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_mucommands DESTINATION ${LC_PLUGINS_DEST})
