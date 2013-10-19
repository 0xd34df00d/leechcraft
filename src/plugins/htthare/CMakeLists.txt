cmake_minimum_required (VERSION 2.8)
project (leechcraft_httthare)
include (InitLCPlugin OPTIONAL)

find_package (Boost REQUIRED COMPONENTS system)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	httthare.cpp
	connection.cpp
	requesthandler.cpp
	)
#CreateTrs("httthare" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("httthare" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_httthare SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_httthare
	${QT_LIBRARIES}
	${Boost_SYSTEM_LIBRARY}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_httthare DESTINATION ${LC_PLUGINS_DEST})
