cmake_minimum_required (VERSION 3.10)
project (lmp_potorchu)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (projectM4 REQUIRED)
find_package (projectM4Playlist REQUIRED)

LC_DEFINE_PLUGIN (
	SRCS
		potorchu.cpp
		visualfilter.cpp
		viswidget.cpp
		visscene.cpp
	QT_COMPONENTS OpenGL OpenGLWidgets
	LINK_LIBRARIES PkgConfig::GStreamer libprojectM::projectM libprojectM::playlist leechcraft-lmp-util
	)
