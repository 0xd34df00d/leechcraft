cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_postrus)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS postrus.cpp redirector.cpp
	QT_COMPONENTS Gui
	INSTALL_DESKTOP
	)
