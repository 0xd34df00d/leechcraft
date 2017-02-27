cmake_minimum_required (VERSION 2.8)
project (leechcraft_touchstreams)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	touchstreams.cpp
	audiosearch.cpp
	xmlsettingsmanager.cpp
	albumsmanager.cpp
	friendsmanager.cpp
	authclosehandler.cpp
	recsmanager.cpp
	util.cpp
	tracksrestorehandler.cpp
	)
set (RESOURCES touchstreamsresources.qrc)

QtAddResources (RCCS ${RESOURCES})

CreateTrs ("touchstreams" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("touchstreams" "en;ru_RU" "${SRCS}" "${FORMS}" "touchstreamssettings.xml")

add_library (leechcraft_touchstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_touchstreams
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_touchstreams DESTINATION ${LC_PLUGINS_DEST})
install (FILES touchstreamssettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_touchstreams Concurrent Network Widgets Xml)
