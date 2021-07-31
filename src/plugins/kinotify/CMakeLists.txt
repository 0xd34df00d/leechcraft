cmake_minimum_required (VERSION 3.10)
project (leechcraft_kinotify)
include (InitLCPlugin NO_POLICY_SCOPE)

if (UNIX AND NOT APPLE)
	set (QT_EXTRA_COMPONENTS X11Extras)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		kinotify.cpp
		kinotifywidget.cpp
		xmlsettingsmanager.cpp
		$<$<PLATFORM_ID:Linux,FreeBSD>:fswinwatcher_x11.cpp>
		$<$<PLATFORM_ID:Windows>:fswinwatcher_win.cpp>
	SETTINGS kinotifysettings.xml
	QT_COMPONENTS ${QT_EXTRA_COMPONENTS}
	INSTALL_SHARE
	)
