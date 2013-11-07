cmake_minimum_required (VERSION 2.8)
project (leechcraft_htthare)
include (InitLCPlugin OPTIONAL)

find_package (Boost REQUIRED COMPONENTS system)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	htthare.cpp
	xmlsettingsmanager.cpp
	server.cpp
	connection.cpp
	requesthandler.cpp
	storagemanager.cpp
	iconresolver.cpp
	trmanager.cpp
	)
CreateTrs("htthare" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("htthare" "en;ru_RU" "${SRCS}" "${FORMS}" "httharesettings.xml")

add_library (leechcraft_htthare SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_htthare
	${QT_LIBRARIES}
	${Boost_SYSTEM_LIBRARY}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_htthare DESTINATION ${LC_PLUGINS_DEST})
install (FILES httharesettings.xml DESTINATION ${LC_SETTINGS_DEST})
