cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_murm)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		murm.cpp
		vkprotocol.cpp
		vkaccount.cpp
		vkconnection.cpp
		vkentry.cpp
		vkmessage.cpp
		photofetcher.cpp
		vcarddialog.cpp
		georesolver.cpp
		xmlsettingsmanager.cpp
		longpollmanager.cpp
		groupsmanager.cpp
		mucjoinwidget.cpp
		entrybase.cpp
		vkchatentry.cpp
		logger.cpp
		accountconfigdialog.cpp
		serverhistorymanager.cpp
		vkconnectiontunesetter.cpp
		transfermanager.cpp
		pendingupload.cpp
		util.cpp
		appinfomanager.cpp
		structures.cpp
		photourlstorage.cpp
		servermessagessyncer.cpp
	SETTINGS azothmurmsettings.xml
	QT_COMPONENTS Concurrent Network Sql Widgets Xml
	INSTALL_SHARE
	)
