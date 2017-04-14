cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle_fxb)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	fxb.cpp
	document.cpp
	fb2converter.cpp
	toclink.cpp
	xmlsettingsmanager.cpp
	)

CreateTrs ("monocle_fxb" "en;ru_RU" FXB_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle_fxb" "en;ru_RU" "${SRCS}" "" "monoclefxbsettings.xml")

add_library (leechcraft_monocle_fxb SHARED
	${SRCS}
	${FXB_COMPILED_TRANSLATIONS}
	)
target_link_libraries (leechcraft_monocle_fxb
	${LEECHCRAFT_LIBRARIES}
	leechcraft_monocle_util
	)
install (TARGETS leechcraft_monocle_fxb DESTINATION ${LC_PLUGINS_DEST})
install (FILES monoclefxbsettings.xml DESTINATION ${LC_SETTINGS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-monocle-fxb-qt5.desktop DESTINATION share/applications)
endif ()

FindQtLibs (leechcraft_monocle_fxb Xml)
