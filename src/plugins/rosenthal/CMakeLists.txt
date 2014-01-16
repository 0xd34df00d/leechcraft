cmake_minimum_required (VERSION 2.8)
project (leechcraft_rosenthal)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	rosenthal.cpp
	)
#CreateTrs("rosenthal" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("rosenthal" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_rosenthal SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_rosenthal
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_rosenthal DESTINATION ${LC_PLUGINS_DEST})
