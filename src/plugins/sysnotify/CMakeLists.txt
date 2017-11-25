cmake_minimum_required (VERSION 3.3)
project (leechcraft_sysnotify)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	sysnotify.cpp
	notificationmanager.cpp
	imagehint.cpp
	)
CreateTrs("sysnotify" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("sysnotify" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_sysnotify SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_sysnotify
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_sysnotify DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_sysnotify DBus Gui)
