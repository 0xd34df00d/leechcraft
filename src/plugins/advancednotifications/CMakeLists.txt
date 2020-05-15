cmake_minimum_required (VERSION 3.3)
project (leechcraft_advancednotifications)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (advancednotifications
	SRCS
		advancednotifications.cpp
		generalhandler.cpp
		concretehandlerbase.cpp
		systemtrayhandler.cpp
		notificationruleswidget.cpp
		xmlsettingsmanager.cpp
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
	FORMS
		notificationruleswidget.ui
		matchconfigdialog.ui
		boolmatcherconfigwidget.ui
		intmatcherconfigwidget.ui
		stringlikematcherconfigwidget.ui
		addfrommisseddialog.ui
	SETTINGS advancednotificationssettings.xml
	QT_COMPONENTS QuickWidgets
	INSTALL_SHARE
	)

option (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE "Enable Dolle, OS X notifications backend" OFF)

if (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE)
	add_subdirectory (plugins/dolle)
endif ()
