cmake_minimum_required (VERSION 3.3)
project (leechcraft_fontiac)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		fontiac.cpp
		substsmanager.cpp
		xmlsettingsmanager.cpp
	SETTINGS fontiacsettings.xml
	QT_COMPONENTS Gui
	)
