cmake_minimum_required (VERSION 2.8)
project (leechcraft_dlniwe)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	dlniwe.cpp
	multicastserver.cpp
	)
#CreateTrs("dlniwe" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("dlniwe" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_dlniwe SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_dlniwe
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_dlniwe RUNTIME DESTINATION ${LC_PLUGINS_DEST})
