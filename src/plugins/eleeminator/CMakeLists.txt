cmake_minimum_required (VERSION 2.8)
project (leechcraft_eleeminator)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	eleeminator.cpp
	termtab.cpp
	)
#CreateTrs("eleeminator" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("eleeminator" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_eleeminator SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_eleeminator
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_eleeminator DESTINATION ${LC_PLUGINS_DEST})
