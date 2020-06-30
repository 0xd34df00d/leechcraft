cmake_minimum_required (VERSION 3.3)
project (leechcraft_scroblibre)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		scroblibre.cpp
		xmlsettingsmanager.cpp
		accountsmanager.cpp
		authmanager.cpp
		singleaccauth.cpp
		util.cpp
		submitinfo.cpp
	SETTINGS scroblibresettings.xml
	QT_COMPONENTS Gui Network
	)
