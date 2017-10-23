cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_standardstyles)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (STANDARDSTYLES_SRCS
	standardstyles.cpp
	standardstylesource.cpp
	)
set (STANDARDSTYLES_RESOURCES azothstandardstylesresources.qrc)

QtAddResources (STANDARDSTYLES_RCCS ${STANDARDSTYLES_RESOURCES})

CreateTrs ("azoth_standardstyles" "en;ru_RU" STANDARDSTYLES_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_standardstyles" "en;ru_RU" "${STANDARDSTYLES_SRCS}" "" "")

add_library (leechcraft_azoth_standardstyles SHARED
	${STANDARDSTYLES_COMPILED_TRANSLATIONS}
	${STANDARDSTYLES_SRCS}
	${STANDARDSTYLES_RCCS}
	)
target_link_libraries (leechcraft_azoth_standardstyles
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_standardstyles DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})

FindQtLibs (leechcraft_azoth_standardstyles WebKitWidgets)
