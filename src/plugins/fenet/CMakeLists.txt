cmake_minimum_required (VERSION 2.8)
project (leechcraft_fenet)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	fenet.cpp
	)
#CreateTrs("fenet" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("fenet" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_fenet SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_fenet
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_fenet DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/xsessions DESTINATION share)
