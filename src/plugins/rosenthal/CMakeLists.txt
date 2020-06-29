cmake_minimum_required (VERSION 3.3)
project (leechcraft_rosenthal)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Hunspell REQUIRED IMPORTED_TARGET hunspell>=1.5.1)

LC_DEFINE_PLUGIN (
	SRCS
		rosenthal.cpp
		checker.cpp
		knowndictsmanager.cpp
		xmlsettingsmanager.cpp
	SETTINGS rosenthalsettings.xml
	QT_COMPONENTS Gui
	LINK_LIBRARIES PkgConfig::Hunspell
	)
