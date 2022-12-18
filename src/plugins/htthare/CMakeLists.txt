cmake_minimum_required (VERSION 3.10)
project (htthare)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		htthare.cpp
		xmlsettingsmanager.cpp
		server.cpp
		connection.cpp
		requesthandler.cpp
		storagemanager.cpp
		iconresolver.cpp
		trmanager.cpp
	QT_COMPONENTS Gui Network
	SETTINGS httharesettings.xml
	)
