cmake_minimum_required (VERSION 3.3)
project (leechcraft_otzerkalu)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	otzerkalu.cpp
	otzerkaludialog.cpp
	otzerkaludownloader.cpp
	)
set (FORMS
	otzerkaludialog.ui
	)
QtWrapUi (UIS_H ${FORMS})

add_library (leechcraft_otzerkalu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_otzerkalu
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_otzerkalu DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_otzerkalu WebKitWidgets)
