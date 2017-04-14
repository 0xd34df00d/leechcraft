cmake_minimum_required (VERSION 2.8)
project (leechcraft_anhero_crashprocess)

find_package (Boost REQUIRED COMPONENTS program_options)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	)
set (SRCS
	main.cpp
	gdblauncher.cpp
	crashdialog.cpp
	highlighter.cpp
	)
set (FORMS
	crashdialog.ui
	)
QtWrapUi (UIS_H "${FORMS}")

set (EXECSUFFIX)
if (NOT APPLE)
	set (EXECSUFFIX "-qt5")
endif ()

add_executable (lc_anhero_crashprocess${EXECSUFFIX} WIN32
	${SRCS}
	${UIS_H}
	)
target_link_libraries (lc_anhero_crashprocess${EXECSUFFIX}
	${LEECHCRAFT_LIBRARIES}
	${Boost_PROGRAM_OPTIONS_LIBRARY}
	)
install (TARGETS lc_anhero_crashprocess${EXECSUFFIX} DESTINATION ${LC_BINDIR})
FindQtLibs (lc_anhero_crashprocess${EXECSUFFIX} Widgets)

if (UNIX AND NOT APPLE)
	add_custom_target (data_lc_anhero_crashprocess_gz${EXECSUFFIX} ALL
		"${CMAKE_CURRENT_SOURCE_DIR}/data/compress" "lc_anhero_crashprocess${EXECSUFFIX}.1"
		DEPENDS data/lc_anhero_crashprocess${EXECSUFFIX}.1
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
		)
	install (FILES data/lc_anhero_crashprocess${EXECSUFFIX}.1.gz DESTINATION share/man/man1)
endif ()
