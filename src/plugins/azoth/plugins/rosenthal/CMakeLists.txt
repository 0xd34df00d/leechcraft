cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_rosenthal)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_rosenthal
	SRCS
		rosenthal.cpp
		highlighter.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
