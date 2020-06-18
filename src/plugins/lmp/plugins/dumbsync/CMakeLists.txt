cmake_minimum_required (VERSION 3.3)
project (leechcraft_lmp_dumbsync)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dumbsync.cpp
		xmlsettingsmanager.cpp
	SETTINGS lmpdumbsyncsettings.xml
	QT_COMPONENTS Concurrent Gui
	)
