cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_keeso)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS keeso.cpp
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
