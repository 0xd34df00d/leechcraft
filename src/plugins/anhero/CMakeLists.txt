cmake_minimum_required (VERSION 2.8)
project (leechcraft_anhero)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${KDE4_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	anhero.cpp
	)
set (RESOURCES anheroresources.qrc)
CreateTrs ("anhero" "ar;de;en;es;fr;it;oc;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
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
	)
install (TARGETS leechcraft_anhero DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})

add_subdirectory(crashprocess)
