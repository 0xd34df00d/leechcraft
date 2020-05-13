cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_depester)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_depester
	SRCS
		depester.cpp
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
