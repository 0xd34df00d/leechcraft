cmake_minimum_required (VERSION 3.3)
project (leechcraft_touchstreams)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		touchstreams.cpp
		audiosearch.cpp
		xmlsettingsmanager.cpp
		albumsmanager.cpp
		friendsmanager.cpp
		authclosehandler.cpp
		recsmanager.cpp
		util.cpp
		tracksrestorehandler.cpp
	SETTINGS touchstreamssettings.xml
	QT_COMPONENTS Concurrent Network Widgets Xml
	)
