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
qt_wrap_ui (UIS_H ${FORMS})

add_executable (lc_anhero_crashprocess WIN32
	${SRCS}
	${UIS_H}
	)
set_target_properties (lc_anhero_crashprocess PROPERTIES OUTPUT_NAME lc_anhero_crashprocess${LC_LIBSUFFIX})
target_link_libraries (lc_anhero_crashprocess ${LEECHCRAFT_LIBRARIES})
install (TARGETS lc_anhero_crashprocess DESTINATION ${LC_BINDIR})
FindQtLibs (lc_anhero_crashprocess Widgets)

if (UNIX AND NOT APPLE)
	if (NOT SKIP_MAN_COMPRESS)
		add_custom_target (data_lc_anhero_crashprocess_gz${LC_LIBSUFFIX} ALL
			"${CMAKE_CURRENT_SOURCE_DIR}/data/compress" "lc_anhero_crashprocess${LC_LIBSUFFIX}.1"
			DEPENDS data/lc_anhero_crashprocess${LC_LIBSUFFIX}.1
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
			)
		install (FILES data/lc_anhero_crashprocess${LC_LIBSUFFIX}.1.gz DESTINATION share/man/man1)
	else ()
		install (FILES data/lc_anhero_crashprocess${LC_LIBSUFFIX}.1 DESTINATION share/man/man1)
	endif ()
endif ()
