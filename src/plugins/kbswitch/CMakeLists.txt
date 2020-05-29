cmake_minimum_required (VERSION 3.3)
project (leechcraft_kbswitch)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (KBSwitchXParts REQUIRED IMPORTED_TARGET xkbfile xcb-xkb)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	kbswitch.cpp
	keyboardlayoutswitcher.cpp
	xmlsettingsmanager.cpp
	kbctl.cpp
	quarkproxy.cpp
	flagiconprovider.cpp
	rulesstorage.cpp
	layoutsconfigwidget.cpp
	optionsconfigwidget.cpp
	)
set (FORMS
	layoutsconfigwidget.ui
	)
set (RESOURCES kbswitchresources.qrc)
QtAddResources (RCCS ${RESOURCES})
QtWrapUi (UIS_H ${FORMS})

CreateTrs ("kbswitch" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("kbswitch" "en;ru_RU" "${SRCS}" "${FORMS}" "kbswitchsettings.xml")

add_library (leechcraft_kbswitch SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${UIS_H}
	)

target_link_libraries (leechcraft_kbswitch
	${LEECHCRAFT_LIBRARIES}
	PkgConfig::KBSwitchXParts
	)
install (TARGETS leechcraft_kbswitch DESTINATION ${LC_PLUGINS_DEST})
install (FILES kbswitchsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml5/kbswitch DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_kbswitch QuickWidgets X11Extras)

include_directories (${Qt5Gui_PRIVATE_INCLUDE_DIRS})
