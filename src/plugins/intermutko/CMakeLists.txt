cmake_minimum_required (VERSION 3.10)
project (intermutko)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		acceptlangwidget.cpp
		addentrydialog.cpp
		entriesdelegate.cpp
		intermutko.cpp
		localeentry.cpp
		localesmodel.cpp
		util.cpp
	SETTINGS intermutkosettings.xml
	QT_COMPONENTS Network Widgets
	)
