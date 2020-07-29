cmake_minimum_required (VERSION 3.10)
project (leechcraft_xtazy)
include (InitLCPlugin NO_POLICY_SCOPE)

option (WITH_XTAZY_DBUS "Enable support for DBus/MPRIS" ON)

if (WITH_XTAZY_DBUS)
	add_definitions (-DHAVE_DBUS)
	set (QT_DBUS_COMPONENT DBus)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		xtazy.cpp
		xmlsettingsmanager.cpp
		tunesourcebase.cpp
		filesource.cpp
		lcsource.cpp
		$<$<BOOL:${WITH_XTAZY_DBUS}>:mprissource.cpp>
	SETTINGS xtazysettings.xml
	QT_COMPONENTS Gui ${QT_DBUS_COMPONENT}
	)
