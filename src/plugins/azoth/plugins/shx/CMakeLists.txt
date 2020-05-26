cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_shx)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		shx.cpp
		xmlsettingsmanager.cpp
	SETTINGS azothshxsettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
