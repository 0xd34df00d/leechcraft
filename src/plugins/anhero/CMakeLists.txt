cmake_minimum_required (VERSION 3.3)
project (leechcraft_anhero)
include (InitLCPlugin)

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
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_anhero SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_anhero
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_anhero DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_anhero Core Gui)

add_subdirectory(crashprocess)
