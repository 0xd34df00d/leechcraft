cmake_minimum_required (VERSION 3.10)
project (azoth_tracolor)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		tracolor.cpp
		entryeventsmanager.cpp
		iconsmanager.cpp
		eventssettingsmanager.cpp
	SETTINGS azothtracolorsettings.xml
	QT_COMPONENTS Widgets
	)
