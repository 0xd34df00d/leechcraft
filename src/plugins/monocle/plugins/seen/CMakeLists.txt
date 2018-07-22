cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_seen)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (DjVuLibre REQUIRED)

include_directories (SYSTEM
	${Boost_INCLUDE_DIR}
	${DJVULIBRE_INCLUDE_DIR}
)
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	seen.cpp
	docmanager.cpp
	document.cpp
	)

CreateTrs ("monocle_seen" "en;ru_RU" SEEN_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle_seen" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_monocle_seen SHARED
	${SEEN_COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_monocle_seen
	${LEECHCRAFT_LIBRARIES}
	${DJVULIBRE_LIBRARY}
	)
install (TARGETS leechcraft_monocle_seen DESTINATION ${LC_PLUGINS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-monocle-seen-qt5.desktop DESTINATION share/applications)
endif ()

FindQtLibs (leechcraft_monocle_seen Gui)
