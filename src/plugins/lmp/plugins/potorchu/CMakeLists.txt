cmake_minimum_required (VERSION 3.10)
project (lmp_potorchu)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (ProjectM REQUIRED IMPORTED_TARGET libprojectM)

LC_DEFINE_PLUGIN (
	SRCS
		potorchu.cpp
		visualfilter.cpp
		viswidget.cpp
		visscene.cpp
	RESOURCES lmppotorchuresources.qrc
	QT_COMPONENTS OpenGL
	LINK_LIBRARIES PkgConfig::GStreamer PkgConfig::ProjectM
	)
