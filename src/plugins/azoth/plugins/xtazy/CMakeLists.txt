cmake_minimum_required (VERSION 3.10)
project (azoth_xtazy)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		xtazy.cpp
		tracksharedialog.cpp
	SETTINGS azothxtazysettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
