cmake_minimum_required (VERSION 2.8)
project (leechcraft_imgaste)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	imgaste.cpp
	)
#CreateTrs("imgaste" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("imgaste" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_imgaste SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_imgaste
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_imgaste DESTINATION ${LC_PLUGINS_DEST})
