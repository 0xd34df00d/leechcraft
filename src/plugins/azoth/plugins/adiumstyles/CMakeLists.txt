cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_adiumstyles)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTXML TRUE)
set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ADIUMSTYLES_SRCS
	adiumstyles.cpp
	adiumstylesource.cpp
	packproxymodel.cpp
	plistparser.cpp
	)
set (ADIUMSTYLES_RESOURCES
	adiumstyles.qrc
	)
QT4_ADD_RESOURCES (ADIUMSTYLES_RCCS ${ADIUMSTYLES_RESOURCES})

CreateTrs ("azoth_adiumstyles" "en;ru_RU" ADIUMSTYLES_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_adiumstyles" "en;ru_RU" "${ADIUMSTYLES_SRCS}" "" "")

add_library (leechcraft_azoth_adiumstyles SHARED
	${ADIUMSTYLES_COMPILED_TRANSLATIONS}
	${ADIUMSTYLES_SRCS}
	${ADIUMSTYLES_RCCS}
	)
target_link_libraries (leechcraft_azoth_adiumstyles
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_adiumstyles DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})
