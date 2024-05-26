cmake_minimum_required (VERSION 3.10)
project (monocle_pdf)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (PopplerQt5 REQUIRED IMPORTED_TARGET poppler-qt5)

LC_DEFINE_PLUGIN (
	SRCS
		pdf.cpp
		document.cpp
		links.cpp
		fields.cpp
		annotations.cpp
		pendingfontinforequest.cpp
	SETTINGS monoclepdfsettings.xml
	QT_COMPONENTS Concurrent Gui Xml
	LINK_LIBRARIES PkgConfig::PopplerQt5 leechcraft-monocle-util
	INSTALL_SHARE
	INSTALL_DESKTOP
	)
