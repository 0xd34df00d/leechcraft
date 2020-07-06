cmake_minimum_required (VERSION 3.3)
project (leechcraft_sysnotify)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		sysnotify.cpp
		notificationmanager.cpp
		imagehint.cpp
	QT_COMPONENTS DBus Gui
	)
