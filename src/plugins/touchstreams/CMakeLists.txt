cmake_minimum_required (VERSION 2.8)
project (leechcraft_touchstreams)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (QJSON REQUIRED)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)
set (SRCS
	touchstreams.cpp
	audiosearch.cpp
	xmlsettingsmanager.cpp
	albumsmanager.cpp
	friendsmanager.cpp
	authclosehandler.cpp
	)
set (RESOURCES touchstreamsresources.qrc)

QT4_ADD_RESOURCES (RCCS ${RESOURCES})

CreateTrs ("touchstreams" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("touchstreams" "en;ru_RU" "${SRCS}" "${FORMS}" "touchstreamssettings.xml")

add_library (leechcraft_touchstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_touchstreams
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_touchstreams DESTINATION ${LC_PLUGINS_DEST})
install (FILES touchstreamssettings.xml DESTINATION ${LC_SETTINGS_DEST})
