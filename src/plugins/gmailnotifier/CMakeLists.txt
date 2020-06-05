cmake_minimum_required (VERSION 3.3)
project (leechcraft_gmailnotifier)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		gmailnotifier.cpp
		xmlsettingsmanager.cpp
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
