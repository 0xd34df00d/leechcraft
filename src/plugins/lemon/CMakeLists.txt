cmake_minimum_required (VERSION 3.10)
project (lemon)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt5Qwt Qt5Qwt6)
pkg_check_modules (LemonLibnl REQUIRED IMPORTED_TARGET libnl-3.0 libnl-route-3.0)

LC_DEFINE_PLUGIN (
	SRCS
		lemon.cpp
		trafficmanager.cpp
		trafficdialog.cpp
		platformbackend.cpp
		linuxplatformbackend.cpp
		quarkproxy.cpp
	SETTINGS lemonsettings.xml
	QT_COMPONENTS Network Widgets
	LINK_LIBRARIES PkgConfig::Qwt PkgConfig::LemonLibnl
	INSTALL_SHARE
	)
