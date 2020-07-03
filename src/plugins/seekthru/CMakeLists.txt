cmake_minimum_required (VERSION 3.3)
project (leechcraft_seekthru)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		seekthru.cpp
		core.cpp
		description.cpp
		xmlsettingsmanager.cpp
		searcherslist.cpp
		findproxy.cpp
		searchhandler.cpp
		wizardgenerator.cpp
		startupfirstpage.cpp
		tagsasker.cpp
	RESOURCES seekthruresources.qrc
	SETTINGS seekthrusettings.xml
	QT_COMPONENTS Widgets Xml
	INSTALL_SHARE
	)
