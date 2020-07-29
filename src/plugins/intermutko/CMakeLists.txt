cmake_minimum_required (VERSION 3.10)
project (leechcraft_intermutko)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		acceptlangwidget.cpp
		entriesdelegate.cpp
		intermutko.cpp
		localeentry.cpp
		localesmodel.cpp
		util.cpp
		xmlsettingsmanager.cpp
	SETTINGS intermutkosettings.xml
	QT_COMPONENTS Network Widgets
	)
