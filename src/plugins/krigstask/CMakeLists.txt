cmake_minimum_required (VERSION 3.10)
project (krigstask)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (KrigstaskXcbParts REQUIRED IMPORTED_TARGET xcomposite xcb-composite xcb-render xcb-renderutil)

LC_DEFINE_PLUGIN (
	SRCS
		krigstask.cpp
		windowsmodel.cpp
		taskbarproxy.cpp
		pagerwindow.cpp
		pagerwindowproxy.cpp
	QT_COMPONENTS QuickWidgets X11Extras
	LINK_LIBRARIES PkgConfig::KrigstaskXcbParts
	INSTALL_SHARE
	)
