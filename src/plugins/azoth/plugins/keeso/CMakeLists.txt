cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_keeso)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
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
QT4_ADD_RESOURCES (KEESO_RCCS ${KEESO_RESOURCES})

add_library (leechcraft_azoth_keeso SHARED
	${KEESO_SRCS}
	${KEESO_RCCS}
	)
target_link_libraries (leechcraft_azoth_keeso
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_keeso DESTINATION ${LC_PLUGINS_DEST})
