cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_postrus)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${LIBSPECTRE_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	postrus.cpp
	redirector.cpp
	)

add_library (leechcraft_monocle_postrus SHARED
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_postrus
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_monocle_postrus DESTINATION ${LC_PLUGINS_DEST})

if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-monocle-postrus.desktop DESTINATION share/applications)
endif ()
