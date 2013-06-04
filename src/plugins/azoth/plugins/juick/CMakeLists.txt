cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_juick)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (JUICK_SRCS
	juick.cpp
	)
set (JUICK_HEADERS
	juick.h
	)
set (JUICK_RESOURCES
	juick.qrc
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (JUICK_MOC_SRCS ${JUICK_HEADERS})
endif (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (JUICK_RCCS ${JUICK_RESOURCES})

add_library (leechcraft_azoth_juick SHARED
	${JUICK_SRCS}
	${JUICK_MOC_SRCS}
	${JUICK_RCCS}
	)
target_link_libraries (leechcraft_azoth_juick
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_juick DESTINATION ${LC_PLUGINS_DEST})
