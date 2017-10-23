cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_nativeemoticons)
include (InitLCPlugin OPTIONAL)

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
QtAddResources (NATIVEEMOTICONS_RCCS ${NATIVEEMOTICONS_RESOURCES})

CreateTrs ("azoth_nativeemoticons" "en;ru_RU" NATIVEEMOTICONS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_nativeemoticons" "en;ru_RU" "${NATIVEEMOTICONS_SRCS}" "" "")

add_library (leechcraft_azoth_nativeemoticons SHARED
	${NATIVEEMOTICONS_COMPILED_TRANSLATIONS}
	${NATIVEEMOTICONS_RCCS}
	${NATIVEEMOTICONS_SRCS}
	)
target_link_libraries (leechcraft_azoth_nativeemoticons
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_nativeemoticons DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})

FindQtLibs (leechcraft_azoth_nativeemoticons Xml Widgets)
