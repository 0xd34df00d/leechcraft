cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_postrus)
include (InitLCPlugin)

include_directories (
	${LIBSPECTRE_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	postrus.cpp
	redirector.cpp
	)

CreateTrs ("monocle_postrus" "en;ru_RU" POSTRUS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle_postrus" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_monocle_postrus SHARED
	${POSTRUS_COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_postrus
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_monocle_postrus DESTINATION ${LC_PLUGINS_DEST})

if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-monocle-postrus-qt5.desktop DESTINATION share/applications)
endif ()

FindQtLibs (leechcraft_monocle_postrus Gui)
