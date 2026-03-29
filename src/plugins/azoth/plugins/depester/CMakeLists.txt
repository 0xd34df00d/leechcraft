cmake_minimum_required (VERSION 3.10)
project (azoth_depester)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS depester.cpp
	QT_COMPONENTS Core Widgets
	LINK_LIBRARIES leechcraft-azoth-util
	INSTALL_SHARE
	)
