cmake_minimum_required (VERSION 3.10)
project (gmailnotifier)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		gmailnotifier.cpp
		gmailchecker.cpp
		convinfo.cpp
		notifier.cpp
		quarkmanager.cpp
		maillistview.cpp
	RESOURCES res/resources.qrc
	SETTINGS gmailnotifiersettings.xml
	QT_COMPONENTS Network QuickWidgets Xml
	INSTALL_SHARE
	)
