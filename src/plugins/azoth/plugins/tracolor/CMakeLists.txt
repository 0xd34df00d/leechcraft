cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_tracolor)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_tracolor
	SRCS
		tracolor.cpp
		entryeventsmanager.cpp
		iconsmanager.cpp
		xmlsettingsmanager.cpp
		eventssettingsmanager.cpp
	SETTINGS azothtracolorsettings.xml
	QT_COMPONENTS Widgets
	)
