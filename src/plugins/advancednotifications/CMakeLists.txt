cmake_minimum_required (VERSION 3.10)
project (advancednotifications)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		advancednotifications.cpp
		generalhandler.cpp
		concretehandlerbase.cpp
		systemtrayhandler.cpp
		notificationruleswidget.cpp
		notificationrule.cpp
		fieldmatch.cpp
		typedmatchers.cpp
		matchconfigdialog.cpp
		visualhandler.cpp
		audiohandler.cpp
		cmdrunhandler.cpp
		enablesoundactionmanager.cpp
		wmurgenthandler.cpp
		rulesmanager.cpp
		quarkproxy.cpp
		actionsmodel.cpp
		visualnotificationsview.cpp
		eventproxyobject.cpp
		actionsproxyobject.cpp
		audiothememanager.cpp
		unhandlednotificationskeeper.cpp
		addfrommisseddialog.cpp
	SETTINGS advancednotificationssettings.xml
	QT_COMPONENTS QuickWidgets Core5Compat
	INSTALL_SHARE
	)

option (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE "Enable Dolle, OS X notifications backend" OFF)

if (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE)
	add_subdirectory (plugins/dolle)
endif ()
