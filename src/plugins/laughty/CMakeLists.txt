cmake_minimum_required (VERSION 3.3)
project (leechcraft_laughty)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	laughty.cpp
	serverobject.cpp
	serveradaptor.cpp
	)
CreateTrs("laughty" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("laughty" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_laughty SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_laughty
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_laughty DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_laughty DBus Gui)
