cmake_minimum_required (VERSION 3.10)
project (leechcraft_knowhow)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		knowhow.cpp
		xmlsettingsmanager.cpp
		tipdialog.cpp
	SETTINGS knowhowsettings.xml
	QT_COMPONENTS Widgets Xml
	INSTALL_SHARE
	)
