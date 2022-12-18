cmake_minimum_required (VERSION 3.10)
project (tabsessmanager)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		tabsessmanager.cpp
		restoresessiondialog.cpp
		sessionmenumanager.cpp
		sessionsmanager.cpp
		tabspropsmanager.cpp
		util.cpp
		unclosemanager.cpp
	QT_COMPONENTS Widgets
	)
