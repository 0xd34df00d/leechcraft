cmake_minimum_required (VERSION 2.8)
project (leechcraft_vrooby)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
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
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrs ("vrooby" "en;ru_RU" COMPILED_TRANSLATIONS)

option (ENABLE_VROOBY_UDISKS "Enable UDisks backend for Vrooby" TRUE)
option (ENABLE_VROOBY_UDISKS2 "Enable UDisks2 backend for Vrooby" FALSE)

if (ENABLE_VROOBY_UDISKS)
	add_subdirectory (backends/udisks)
	add_definitions (-DENABLE_UDISKS)
endif (ENABLE_VROOBY_UDISKS)

if (ENABLE_VROOBY_UDISKS2)
	add_subdirectory (backends/udisks2)
	add_definitions (-DENABLE_UDISKS2)
endif (ENABLE_VROOBY_UDISKS2)

add_library (leechcraft_vrooby SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_vrooby
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}

	${UDISKS_BACKEND_LIBS}
	)
install (TARGETS leechcraft_vrooby DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
