cmake_minimum_required (VERSION 3.10)
project (azoth_chathistory)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		chathistory.cpp
		chathistorywidget.cpp
		chatfindbox.cpp
		historymessage.cpp
		historyvieweventfilter.cpp
		loggingstatekeeper.cpp
		searchhandler.cpp
		storage2.cpp
	SETTINGS azothchathistorysettings.xml
	QT_COMPONENTS Core Sql Widgets
	INSTALL_SHARE
	)
