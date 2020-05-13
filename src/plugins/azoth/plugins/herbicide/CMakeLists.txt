cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_herbicide)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_herbicide
	SRCS
		herbicide.cpp
		confwidget.cpp
		xmlsettingsmanager.cpp
		logger.cpp
		listsholder.cpp
	FORMS confwidget.ui
	SETTINGS azothherbicidesettings.xml
	QT_COMPONENTS Sql Widgets
	INSTALL_SHARE
	)
