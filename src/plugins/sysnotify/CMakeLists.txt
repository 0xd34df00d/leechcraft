cmake_minimum_required (VERSION 2.8)
project (leechcraft_sysnotify)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDBUS TRUE)
include (${QT_USE_FILE})
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
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_sysnotify DESTINATION ${LC_PLUGINS_DEST})
