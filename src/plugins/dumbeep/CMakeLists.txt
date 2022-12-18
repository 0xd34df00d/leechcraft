cmake_minimum_required (VERSION 3.10)
project (dumbeep)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dumbeep.cpp
		xmlsettingsmanager.cpp
	SETTINGS dumbeepsettings.xml
	QT_COMPONENTS Core
	)
