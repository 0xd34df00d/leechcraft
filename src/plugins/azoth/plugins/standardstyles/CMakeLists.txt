cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_standardstyles)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})

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

QT4_ADD_RESOURCES (STANDARDSTYLES_RCCS ${STANDARDSTYLES_RESOURCES})

add_library (leechcraft_azoth_standardstyles SHARED
	${STANDARDSTYLES_SRCS}
	${STANDARDSTYLES_RCCS}
	)
target_link_libraries (leechcraft_azoth_standardstyles
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_standardstyles DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})
