cmake_minimum_required (VERSION 3.3)
project (leechcraft_xproxy)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		xproxy.cpp
		proxyfactory.cpp
		xmlsettingsmanager.cpp
		proxiesconfigwidget.cpp
		proxyconfigdialog.cpp
		proxiesstorage.cpp
		structures.cpp
		editurlsdialog.cpp
		editurldialog.cpp
		editlistsdialog.cpp
		scriptsmanager.cpp
		urllistscript.cpp
	SETTINGS xproxysettings.xml
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
