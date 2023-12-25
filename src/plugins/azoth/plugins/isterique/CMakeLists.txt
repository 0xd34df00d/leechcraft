cmake_minimum_required (VERSION 3.10)
project (azoth_isterique)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		isterique.cpp
	SETTINGS azothisteriquesettings.xml
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
