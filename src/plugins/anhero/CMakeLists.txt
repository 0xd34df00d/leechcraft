cmake_minimum_required (VERSION 2.8)
project (leechcraft_anhero)
include (InitLCPlugin OPTIONAL)

option (WITH_X11 "Enable X11 support on Linux" ON)
if (WITH_X11 AND NOT APPLE)
	find_package (X11)

	if (X11_FOUND)
		add_definitions (-DHAVE_X11)
		set (ADDITIONAL_LIBS ${X11_X11_LIB})
	endif ()
endif ()

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	anhero.cpp
	)
set (RESOURCES anheroresources.qrc)
CreateTrs ("anhero" "ar;de;en;es;fr;it;oc;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("anhero" "en;ru_RU" "${SRCS}" "${FORMS}" "")
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_anhero SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_anhero
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
install (TARGETS leechcraft_anhero DESTINATION ${LC_PLUGINS_DEST})

add_subdirectory(crashprocess)
