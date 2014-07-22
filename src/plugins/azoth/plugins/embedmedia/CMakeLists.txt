cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_embedmedia)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (EMBEDMEDIA_SRCS
	embedmedia.cpp
	)
set (EMBEDMEDIA_RESOURCES
	embedmedia.qrc
	)
QT4_ADD_RESOURCES (EMBEDMEDIA_RCCS ${EMBEDMEDIA_RESOURCES})

add_library (leechcraft_azoth_embedmedia SHARED
    ${EMBEDMEDIA_SRCS}
    ${EMBEDMEDIA_RCCS}
    )
target_link_libraries (leechcraft_azoth_embedmedia
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_embedmedia DESTINATION ${LC_PLUGINS_DEST})
