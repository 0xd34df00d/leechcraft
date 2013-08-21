cmake_minimum_required (VERSION 2.8)
project (leechcraft_vlc)
include (InitLCPlugin OPTIONAL)

#find_package (VLC REQUIRED)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	vlc.cpp
	vlcwidget.cpp
	vlcplayer.cpp
	)
set (FORMS
	vlcinterface.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})
	
#CreateTrs("vlc" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("vlc" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_vlc SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_vlc
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	/usr/lib/libvlc.so
	)
install (TARGETS leechcraft_vlc DESTINATION ${LC_PLUGINS_DEST})
