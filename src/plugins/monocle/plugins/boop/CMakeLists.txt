cmake_minimum_required (VERSION 3.10)
project (monocle_boop)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (QuaZip REQUIRED IMPORTED_TARGET quazip1-qt${LC_QT_VERSION})

LC_DEFINE_PLUGIN (
	SRCS
		boop.cpp
		document.cpp
		epubloader.cpp
		internallinks.cpp
		manifest.cpp
		microcsshandler.cpp
		microcssparser.cpp
		toc.cpp
		util.cpp
	QT_COMPONENTS Gui Xml
	LINK_LIBRARIES PkgConfig::QuaZip leechcraft-monocle-util
	INSTALL_DESKTOP
	)
