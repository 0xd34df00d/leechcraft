cmake_minimum_required (VERSION 3.3)
project (leechcraft_dumbeep)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dumbeep.cpp
		xmlsettingsmanager.cpp
	SETTINGS dumbeepsettings.xml
	QT_COMPONENTS Core
	)
