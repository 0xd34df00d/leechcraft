cmake_minimum_required (VERSION 3.10)
project (leechcraft_azoth_hili)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		hili.cpp
		xmlsettingsmanager.cpp
	SETTINGS azothhilisettings.xml
	QT_COMPONENTS Core Widgets
	INSTALL_SHARE
	)
