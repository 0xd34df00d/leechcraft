cmake_minimum_required (VERSION 3.3)
project (leechcraft_lemon)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Qwt REQUIRED IMPORTED_TARGET Qt5Qwt)
pkg_check_modules (LemonLibnl REQUIRED IMPORTED_TARGET libnl-3.0 libnl-route-3.0)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
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
	PkgConfig::Qwt
	PkgConfig::LemonLibnl
	)
install (TARGETS leechcraft_lemon DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/lemon DESTINATION ${LC_QML5_DEST})
install (FILES lemonsettings.xml DESTINATION ${LC_SETTINGS_DEST})

CreateTrsQml("lemon" "en;ru_RU" leechcraft_lemon "share/qml5/lemon")

FindQtLibs (leechcraft_lemon Network Widgets)
