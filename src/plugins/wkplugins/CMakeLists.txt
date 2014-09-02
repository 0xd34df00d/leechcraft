cmake_minimum_required (VERSION 2.8)
project (leechcraft_wkplugins)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	wkplugins.cpp
	)
#CreateTrs("wkplugins" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("wkplugins" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_wkplugins SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_wkplugins
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_wkplugins DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_wkplugins WebKitWidgets)
