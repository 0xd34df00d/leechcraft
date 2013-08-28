cmake_minimum_required (VERSION 2.8)
project (leechcraft_touchstreams)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	touchstreams.cpp
	audiosearch.cpp
	xmlsettingsmanager.cpp
	)

CreateTrs ("touchstreams" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("touchstreams" "en;ru_RU" "${SRCS}" "${FORMS}" "touchstreamssettings.xml")

add_library (leechcraft_touchstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_touchstreams
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_touchstreams RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES touchstreamssettings.xml DESTINATION ${LC_SETTINGS_DEST})
