cmake_minimum_required (VERSION 2.8)
project (leechcraft_namauth)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	namauth.cpp
	)
#CreateTrs("namauth" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("namauth" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_namauth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_namauth
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_namauth DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_namauth Gui)
