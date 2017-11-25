cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_keeso)
include (InitLCPlugin)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (KEESO_SRCS
	keeso.cpp
	)
set (KEESO_RESOURCES
	azothkeesoresources.qrc
	)
QtAddResources (KEESO_RCCS ${KEESO_RESOURCES})

add_library (leechcraft_azoth_keeso SHARED
	${KEESO_SRCS}
	${KEESO_RCCS}
	)
target_link_libraries (leechcraft_azoth_keeso
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_keeso DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_azoth_keeso Core Widgets)
