cmake_minimum_required (VERSION 3.10)
project (sysnotify)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		sysnotify.cpp
		notificationmanager.cpp
		imagehint.cpp
		bodymarkup.cpp
	QT_COMPONENTS DBus Gui
	HAS_TESTS
	)

AddSysnotifyTest (bodymarkup
	SRCS
		tests/bodymarkuptest.cpp
		bodymarkup.cpp
	QT_COMPONENTS Gui
	)
