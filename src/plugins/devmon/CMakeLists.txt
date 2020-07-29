cmake_minimum_required (VERSION 3.10)
project (leechcraft_devmon)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (UDev REQUIRED IMPORTED_TARGET libudev)

LC_DEFINE_PLUGIN (
	SRCS
		devmon.cpp
		udevbackend.cpp
	QT_COMPONENTS Gui
	LINK_LIBRARIES PkgConfig::UDev
	)
