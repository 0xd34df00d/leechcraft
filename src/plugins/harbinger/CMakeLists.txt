cmake_minimum_required (VERSION 3.3)
project (leechcraft_harbinger)
include (InitLCPlugin)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	harbinger.cpp
	)
#CreateTrs("harbinger" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("harbinger" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_harbinger SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_harbinger
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_harbinger DESTINATION ${LC_PLUGINS_DEST})
