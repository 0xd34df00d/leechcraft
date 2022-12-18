cmake_minimum_required (VERSION 3.10)
project (lmp_fradj)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt5Qwt Qt5Qwt6)

LC_DEFINE_PLUGIN (
	SRCS
		fradj.cpp
		eq10bandeffect.cpp
		eqconfigurator.cpp
		eqconfiguratordialog.cpp
		eqbandwidget.cpp
		bandinfo.cpp
	QT_COMPONENTS Widgets
	LINK_LIBRARIES PkgConfig::GStreamer PkgConfig::Qwt
	)
