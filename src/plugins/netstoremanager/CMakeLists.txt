cmake_minimum_required (VERSION 3.10)
project (netstoremanager)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (Boost REQUIRED COMPONENTS container)

LC_DEFINE_PLUGIN (
	SRCS
		netstoremanager.cpp
		managertab.cpp
		xmlsettingsmanager.cpp
		accountsmanager.cpp
		accountslistwidget.cpp
		addaccountdialog.cpp
		upmanager.cpp
		filesproxymodel.cpp
		filestreemodel.cpp
		filesview.cpp
		remotedirectoryselectdialog.cpp
		syncer.cpp
		syncmanager.cpp
		syncwidget.cpp
		syncitemdelegate.cpp
		directorywidget.cpp
		fileswatcherbase.cpp
		utils.cpp
		downmanager.cpp
		$<IF:$<PLATFORM_ID:Linux>,
			fileswatcher_inotify.cpp,
			fileswatcher_dummy.cpp
			>
	SETTINGS netstoremanagersettings.xml
	QT_COMPONENTS Network Widgets
	LINK_LIBRARIES Boost::container
	INSTALL_SHARE
	)

SUBPLUGIN (GOOGLEDRIVE "Build support for Google Drive")
SUBPLUGIN (DROPBOX "Build support for DropBox")
