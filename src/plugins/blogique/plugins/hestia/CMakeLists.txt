cmake_minimum_required (VERSION 3.3)
project (leechcraft_blogique_hestia)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		accountconfigurationdialog.cpp
		accountconfigurationwidget.cpp
		accountstorage.cpp
		hestia.cpp
		localblogaccount.cpp
		localbloggingplatform.cpp
		xmlsettingsmanager.cpp
	SETTINGS blogiquehestiasettings.xml
	QT_COMPONENTS Sql Widgets
	)
