cmake_minimum_required (VERSION 3.3)
project (leechcraft_vtyulc)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (VLC REQUIRED)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${VLC_INCLUDE_DIRS}
	)
set (SRCS
	vlc.cpp
	playlistwidget.cpp
	vlcwidget.cpp
	vlcplayer.cpp
	vlcscrollbar.cpp
	signalledwidget.cpp
	soundwidget.cpp
	playlistmodel.cpp
	xmlsettingsmanager.cpp
	playlisttitlewidget.cpp
	volumenotification.cpp
	)
	
CreateTrs("vtyulc" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("vtyulc" "en;ru_RU" "${SRCS}" "${FORMS}" "vtyulcsettings.xml")

add_library (leechcraft_vtyulc SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
	
target_link_libraries (leechcraft_vtyulc
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${VLC_LIBRARIES}
	)
	
install (TARGETS leechcraft_vtyulc DESTINATION ${LC_PLUGINS_DEST})
install (FILES vtyulcsettings.xml DESTINATION ${LC_SETTINGS_DEST})
