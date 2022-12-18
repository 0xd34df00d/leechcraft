cmake_minimum_required (VERSION 3.10)
project (azoth_rosenthal)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		rosenthal.cpp
		highlighter.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
