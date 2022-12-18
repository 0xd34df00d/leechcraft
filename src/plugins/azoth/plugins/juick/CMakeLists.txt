cmake_minimum_required (VERSION 3.10)
project (azoth_juick)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS juick.cpp
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
