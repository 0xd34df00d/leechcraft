cmake_minimum_required (VERSION 3.10)
project (azoth_herbicide)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		herbicide.cpp
		confwidget.cpp
		logger.cpp
		listsholder.cpp
	SETTINGS azothherbicidesettings.xml
	QT_COMPONENTS Sql Widgets
	INSTALL_SHARE
	)
