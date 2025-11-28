cmake_minimum_required (VERSION 3.10)
project (eleeminator)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (QTermWidget REQUIRED IMPORTED_TARGET qtermwidget${LC_QT_VERSION})

LC_DEFINE_PLUGIN (
	SRCS
		eleeminator.cpp
		termtab.cpp
		processgraphbuilder.cpp
		processinfo.cpp
		closedialog.cpp
		colorschemesmanager.cpp
		shortcutpasser.cpp
		termcolorschemechooser.cpp
		termcontextmenubuilder.cpp
		termfontchooser.cpp
		termtitleupdater.cpp
	SETTINGS eleeminatorsettings.xml
	QT_COMPONENTS Widgets
	LINK_LIBRARIES PkgConfig::QTermWidget
	INSTALL_SHARE
	)
