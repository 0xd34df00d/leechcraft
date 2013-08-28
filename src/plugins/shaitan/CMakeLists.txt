cmake_minimum_required (VERSION 2.8)
project (leechcraft_shaitan)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	shaitan.cpp
	terminalwidget.cpp
	)
set (RESOURCES shaitanresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_shaitan SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_shaitan
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_shaitan DESTINATION ${LC_PLUGINS_DEST})
