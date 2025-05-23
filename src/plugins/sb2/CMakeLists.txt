cmake_minimum_required (VERSION 3.10)
project (sb2)
include (InitLCPlugin NO_POLICY_SCOPE)
include (CMakeDependentOption)

cmake_dependent_option (WITH_X11 "Enable X11 support on Linux" ON "UNIX AND NOT APPLE" OFF)

if (WITH_X11)
	add_definitions (-DWITH_X11)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		sb2.cpp
		sbview.cpp
		viewmanager.cpp
		desaturateeffect.cpp
		quarkproxy.cpp
		quarkmanager.cpp
		quarksettingsmanager.cpp
		tablistview.cpp
		tabunhidelistview.cpp
		launcherdroparea.cpp
		launchercomponent.cpp
		lcmenucomponent.cpp
		traycomponent.cpp
		baseactioncomponent.cpp
		dockactioncomponent.cpp
		quarkunhidelistview.cpp
		quarkorderview.cpp
		declarativewindow.cpp
		viewgeometrymanager.cpp
		viewsettingsmanager.cpp
		sb2util.cpp
		viewpropsmanager.cpp
		dirwatcher.cpp
		manifest.cpp
		panelsettingsdialog.cpp
	SETTINGS sb2panelsettings.xml
	QT_COMPONENTS Quick QuickWidgets Widgets
	INSTALL_SHARE
	)
