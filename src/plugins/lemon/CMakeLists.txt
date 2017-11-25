cmake_minimum_required (VERSION 3.3)
project (leechcraft_lemon)
include (InitLCPlugin)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")
find_package (Libnl REQUIRED)
find_package (Qwt REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LIBNL_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)
set (SRCS
	lemon.cpp
	core.cpp
	trafficmanager.cpp
	trafficdialog.cpp
	platformbackend.cpp
	linuxplatformbackend.cpp
	xmlsettingsmanager.cpp
	quarkproxy.cpp
	)
set (FORMS
	trafficdialog.ui
	)
QtWrapUi (UIS_H ${FORMS})

CreateTrs("lemon" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lemon" "en;ru_RU" "${SRCS}" "" "lemonsettings.xml")

file(GLOB QML_FILES "share/qml5/lemon/*.qml")
CreateQmlTrsUpTarget("lemon" "en;ru_RU" "${QML_FILES}" "share/qml5/lemon/LemonQuark.qml.settings")

add_library (leechcraft_lemon SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_lemon
	${LEECHCRAFT_LIBRARIES}
	${LIBNL_LIBRARIES}
	${QWT_LIBRARIES}
	)
install (TARGETS leechcraft_lemon DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/lemon DESTINATION ${LC_QML5_DEST})
install (FILES lemonsettings.xml DESTINATION ${LC_SETTINGS_DEST})

CreateTrsQml("lemon" "en;ru_RU" leechcraft_lemon "share/qml5/lemon")

FindQtLibs (leechcraft_lemon Network Widgets)
