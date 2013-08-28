cmake_minimum_required (VERSION 2.8)
project (leechcraft_tpi)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	tpi.cpp
	infomodelmanager.cpp
	tooltipview.cpp
	)

add_library (leechcraft_tpi SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_tpi
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_tpi DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml/tpi DESTINATION ${LC_QML_DEST})
