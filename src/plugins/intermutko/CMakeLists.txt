cmake_minimum_required (VERSION 2.8)
project (leechcraft_intermutko)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	intermutko.cpp
	)
#CreateTrs("intermutko" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("intermutko" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_intermutko SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_intermutko
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_intermutko DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_intermutko Core Network)
