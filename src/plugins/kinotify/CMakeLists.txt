cmake_minimum_required (VERSION 3.10)
project (kinotify)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		kinotify.cpp
		kinotifywidget.cpp
		$<$<PLATFORM_ID:Linux,FreeBSD>:fswinwatcher_x11.cpp>
		$<$<PLATFORM_ID:Windows>:fswinwatcher_win.cpp>
	SETTINGS kinotifysettings.xml
	QT_COMPONENTS StateMachine
	INSTALL_SHARE
	)
