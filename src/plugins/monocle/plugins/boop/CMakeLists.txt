cmake_minimum_required (VERSION 3.10)
project (monocle_boop)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (QuaZip REQUIRED IMPORTED_TARGET quazip1-qt5)

LC_DEFINE_PLUGIN (
	SRCS
		boop.cpp
		document.cpp
		epubloader.cpp
		microcssparser.cpp
	QT_COMPONENTS Gui Xml
	LINK_LIBRARIES PkgConfig::QuaZip leechcraft-monocle-util
	)
