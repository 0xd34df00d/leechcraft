cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_juick)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_juick
	SRCS
		juick.cpp
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
