cmake_minimum_required (VERSION 2.8)
project (leechcraft_tpi)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	tpi.cpp
	infomodelmanager.cpp
	)

add_library (leechcraft_tpi SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_tpi
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_tpi DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/tpi DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_tpi Gui)
