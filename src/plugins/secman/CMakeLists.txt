cmake_minimum_required (VERSION 3.3)
project (leechcraft_secman)
include (InitLCPlugin OPTIONAL)

option (TESTS_SECMAN "Enable SecMan tests")

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

option (ENABLE_SECMAN_SECURESTORAGE "Enable SecMan SecureStorage, the advanced encrypted private data storage" OFF)
option (ENABLE_SECMAN_SIMPLESTORAGE "Enable SecMan SimpleStorage, the basic unencrypted private data storage" ON)

option (WITH_SECMAN_EXPOSE_CONTENTSDISPLAY "Build the GUI for viewing the data stored in SecMan" OFF)

if (WITH_SECMAN_EXPOSE_CONTENTSDISPLAY)
	add_definitions (-DSECMAN_EXPOSE_CONTENTSDISPLAY)
endif ()

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
	persistentstorage.cpp
	)
set (FORMS
	contentsdisplaydialog.ui
	)
set (RESOURCES
	secmanresources.qrc
	)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

CreateTrs("secman" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("secman" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_secman SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_secman
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_secman DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY interfaces DESTINATION "include/leechcraft")

FindQtLibs (leechcraft_secman Widgets)
