cmake_minimum_required (VERSION 3.10)
project (kbswitch)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (KBSwitchXParts REQUIRED IMPORTED_TARGET xkbfile xcb-xkb)

LC_DEFINE_PLUGIN (
	SRCS
		kbswitch.cpp
		keyboardlayoutswitcher.cpp
		kbctl.cpp
		quarkproxy.cpp
		flagiconprovider.cpp
		rulesstorage.cpp
		layoutsconfigwidget.cpp
		optionsconfigwidget.cpp
	SETTINGS kbswitchsettings.xml
	QT_COMPONENTS QuickWidgets
	LINK_LIBRARIES PkgConfig::KBSwitchXParts
	INSTALL_SHARE
	)
