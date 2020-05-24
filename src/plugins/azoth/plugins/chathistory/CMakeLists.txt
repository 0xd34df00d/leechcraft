cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_chathistory)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_chathistory
	SRCS
		chathistory.cpp
		storage.cpp
		chathistorywidget.cpp
		historymessage.cpp
		chatfindbox.cpp
		xmlsettingsmanager.cpp
		historyvieweventfilter.cpp
		storagemanager.cpp
		loggingstatekeeper.cpp
	SETTINGS azothchathistorysettings.xml
	QT_COMPONENTS Core Sql Widgets
	INSTALL_SHARE
	)
