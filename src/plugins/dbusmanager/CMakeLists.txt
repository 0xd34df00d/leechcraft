cmake_minimum_required (VERSION 3.3)
project (leechcraft_dbusmanager)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dbusmanager.cpp
		core.cpp
		iinfoadaptor.cpp
		general.cpp
		generaladaptor.cpp
		tasks.cpp
		tasksadaptor.cpp
		webfilestorageadaptor.cpp
	SETTINGS azothherbicidesettings.xml
	QT_COMPONENTS DBus Gui
	INSTALL_SHARE
	)
