cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_fxb)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	fxb.cpp
	document.cpp
	documentadapter.cpp
	fb2converter.cpp
	toclink.cpp
	)

add_library (leechcraft_monocle_fxb SHARED
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_fxb
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_monocle_fxb DESTINATION ${LC_PLUGINS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-monocle-fxb.desktop DESTINATION share/applications)
endif ()
