cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_dik)
include (InitLCPlugin)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	dik.cpp
	mobiparser.cpp
	decompressor.cpp
	util.cpp
	document.cpp
	)

CreateTrs ("monocle_dik" "en;ru_RU" DIK_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle_dik" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_monocle_dik SHARED
	${DIK_COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_dik
	${LEECHCRAFT_LIBRARIES}
	leechcraft_monocle_util
	)
install (TARGETS leechcraft_monocle_dik DESTINATION ${LC_PLUGINS_DEST})
#if (UNIX AND NOT APPLE)
#	install (FILES freedesktop/leechcraft-monocle-dik.desktop DESTINATION share/applications)
#endif ()

FindQtLibs (leechcraft_monocle_dik Gui Xml)
