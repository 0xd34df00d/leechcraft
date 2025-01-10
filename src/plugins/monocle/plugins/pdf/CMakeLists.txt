cmake_minimum_required (VERSION 3.10)
project (monocle_pdf)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (PopplerQt REQUIRED IMPORTED_TARGET poppler-qt${LC_QT_VERSION})

LC_DEFINE_PLUGIN (
	SRCS
		pdf.cpp
		document.cpp
		links.cpp
		fields.cpp
		annotations.cpp
	SETTINGS monoclepdfsettings.xml
	QT_COMPONENTS Concurrent Gui Xml
	LINK_LIBRARIES PkgConfig::PopplerQt leechcraft-monocle-util
	INSTALL_SHARE
	INSTALL_DESKTOP
	)
