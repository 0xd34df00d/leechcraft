cmake_minimum_required (VERSION 3.10)
if (POLICY CMP0071)
	cmake_policy (SET CMP0071 NEW)
endif ()
project (anhero_crashprocess)

include_directories (
	${LEECHCRAFT_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
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
	)
install (TARGETS lc_anhero_crashprocess${EXECSUFFIX} DESTINATION ${LC_BINDIR})
FindQtLibs (lc_anhero_crashprocess${EXECSUFFIX} Widgets)

if (UNIX AND NOT APPLE)
	if (NOT SKIP_MAN_COMPRESS)
		add_custom_target (data_lc_anhero_crashprocess_gz${EXECSUFFIX} ALL
			"${CMAKE_CURRENT_SOURCE_DIR}/data/compress" "lc_anhero_crashprocess${EXECSUFFIX}.1"
			DEPENDS data/lc_anhero_crashprocess${EXECSUFFIX}.1
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
			)
		install (FILES data/lc_anhero_crashprocess${EXECSUFFIX}.1.gz DESTINATION share/man/man1)
	else ()
		install (FILES data/lc_anhero_crashprocess${EXECSUFFIX}.1 DESTINATION share/man/man1)
	endif ()
endif ()
