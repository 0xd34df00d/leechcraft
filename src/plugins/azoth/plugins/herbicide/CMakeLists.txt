cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_herbicide)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		herbicide.cpp
		confwidget.cpp
		xmlsettingsmanager.cpp
		logger.cpp
		listsholder.cpp
	SETTINGS azothherbicidesettings.xml
	QT_COMPONENTS Sql Widgets
	INSTALL_SHARE
	)
