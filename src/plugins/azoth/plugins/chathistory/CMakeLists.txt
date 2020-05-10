cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_chathistory)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (${AZOTH_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

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
	FORMS chathistorywidget.ui
	SETTINGS azothchathistorysettings.xml
	QT_COMPONENTS Core Sql Widgets
	TS_LANGUAGES en es ru_RU uk_UA
	INSTALL_SHARE
	)
