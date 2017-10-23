cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_embedmedia)
include (InitLCPlugin OPTIONAL)

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
QtAddResources (EMBEDMEDIA_RCCS ${EMBEDMEDIA_RESOURCES})

add_library (leechcraft_azoth_embedmedia SHARED
    ${EMBEDMEDIA_SRCS}
    ${EMBEDMEDIA_RCCS}
    )
target_link_libraries (leechcraft_azoth_embedmedia
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_embedmedia DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_embedmedia Core WebKitWidgets)
