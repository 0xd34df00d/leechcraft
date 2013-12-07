cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_nativeemoticons)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (NATIVEEMOTICONS_SRCS
	nativeemoticons.cpp
	nativeemoticonssource.cpp
	kopeteemoticonssource.cpp
	psiplusemoticonssource.cpp
	baseemoticonssource.cpp
	)

set (NATIVEEMOTICONS_RESOURCES nativeemoticonsresources.qrc)
QT4_ADD_RESOURCES (NATIVEEMOTICONS_RCCS ${NATIVEEMOTICONS_RESOURCES})

CreateTrsUpTarget ("azoth_nativeemoticons" "en;ru_RU" "${NATIVEEMOTICONS_SRCS}" "" "")

add_library (leechcraft_azoth_nativeemoticons SHARED
	${NATIVEEMOTICONS_RCCS}
	${NATIVEEMOTICONS_SRCS}
	)
target_link_libraries (leechcraft_azoth_nativeemoticons
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_nativeemoticons DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})
