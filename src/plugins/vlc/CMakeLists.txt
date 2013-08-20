cmake_minimum_required (VERSION 2.8)
project (leechcraft_vlc)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	vlc.cpp
	)
#CreateTrs("vlc" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("vlc" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_vlc SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_vlc
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_vlc DESTINATION ${LC_PLUGINS_DEST})
