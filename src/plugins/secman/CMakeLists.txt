cmake_minimum_required (VERSION 2.8)
project (leechcraft_secman)
include (InitLCPlugin OPTIONAL)

option (TESTS_SECMAN "Enable SecMan tests")

if (TESTS_SECMAN)
	set (QT_USE_QTTEST TRUE)
endif ()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

option (ENABLE_SECMAN_SECURESTORAGE "Enable SecMan SecureStorage, the advanced encrypted private data storage" OFF)
option (ENABLE_SECMAN_SIMPLESTORAGE "Enable SecMan SimpleStorage, the basic unencrypted private data storage" ON)

if (ENABLE_SECMAN_SECURESTORAGE)
	add_subdirectory (plugins/securestorage)
endif ()
if (ENABLE_SECMAN_SIMPLESTORAGE)
	add_subdirectory (plugins/simplestorage)
endif ()

set (SRCS
	secman.cpp
	core.cpp
	contentsdisplaydialog.cpp
	)
set (FORMS
	contentsdisplaydialog.ui
	)
set (RESOURCES
	secmanresources.qrc
	)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_secman SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

FindQtLibs (leechcraft_secman Core)

target_link_libraries (leechcraft_secman
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_secman DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY interfaces DESTINATION "include/leechcraft")
