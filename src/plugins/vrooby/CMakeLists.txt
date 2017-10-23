cmake_minimum_required (VERSION 3.3)
project (leechcraft_vrooby)
include (InitLCPlugin OPTIONAL)
include (CMakeDependentOption)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	vrooby.cpp
	devbackend.cpp
	trayview.cpp
	flatmountableitems.cpp
	)
set (RESOURCES vroobyresources.qrc)
QtAddResources (RCCS ${RESOURCES})
CreateTrs ("vrooby" "en;ru_RU" COMPILED_TRANSLATIONS)

cmake_dependent_option (ENABLE_VROOBY_UDISKS "Enable UDisks backend for Vrooby" ON "NOT WIN32" OFF)
cmake_dependent_option (ENABLE_VROOBY_UDISKS2 "Enable UDisks2 backend for Vrooby" ON "NOT WIN32" OFF)

if (ENABLE_VROOBY_UDISKS)
	add_subdirectory (backends/udisks)
	add_definitions (-DENABLE_UDISKS)
endif ()

if (ENABLE_VROOBY_UDISKS2)
	add_subdirectory (backends/udisks2)
	add_definitions (-DENABLE_UDISKS2)
endif ()

add_library (leechcraft_vrooby SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_vrooby
	${LEECHCRAFT_LIBRARIES}
	${UDISKS_BACKEND_LIBS}
	)
install (TARGETS leechcraft_vrooby DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/vrooby DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_vrooby DBus QuickWidgets)
