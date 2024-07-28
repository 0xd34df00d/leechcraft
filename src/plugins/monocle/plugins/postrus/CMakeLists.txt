cmake_minimum_required (VERSION 3.10)
project (monocle_postrus)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS postrus.cpp
	QT_COMPONENTS Gui
	INSTALL_DESKTOP
	)
