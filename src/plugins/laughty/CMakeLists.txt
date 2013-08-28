cmake_minimum_required (VERSION 2.8)
project (leechcraft_laughty)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDBUS TRUE)
include (${QT_USE_FILE})
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
#CreateTrs("laughty" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("laughty" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_laughty SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_laughty
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_laughty RUNTIME DESTINATION ${LC_PLUGINS_DEST})
