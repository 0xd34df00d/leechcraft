cmake_minimum_required (VERSION 3.3)
project (leechcraft_ooronee)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		ooronee.cpp
		droparea.cpp
		quarkproxy.cpp
		xmlsettingsmanager.cpp
	SETTINGS ooroneesettings.xml
	QT_COMPONENTS Quick Widgets
	INSTALL_SHARE
	)
