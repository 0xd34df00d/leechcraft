cmake_minimum_required (VERSION 3.10)
project (monocle_seen)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (DjVuLibre REQUIRED IMPORTED_TARGET ddjvuapi)

LC_DEFINE_PLUGIN (
	SRCS
		seen.cpp
		docmanager.cpp
		document.cpp
	QT_COMPONENTS Concurrent Gui
	LINK_LIBRARIES PkgConfig::DjVuLibre leechcraft-monocle-util
	INSTALL_DESKTOP
	)
