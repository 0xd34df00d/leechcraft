cmake_minimum_required (VERSION 3.10)
project (liznoo)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt${LC_QT_VERSION}Qwt6 Qt${LC_QT_VERSION}Qwt)

if (NOT WIN32 AND NOT APPLE)
	set (LIZNOO_DBUS_QT_DEP DBus)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		liznoo.cpp
		batteryinfo.cpp
		batteryhistory.cpp
		batteryhistorydialog.cpp
		platformobjects.cpp
		quarkmanager.cpp
		platform/battery/platform.cpp
		platform/events/platform.cpp
		platform/poweractions/platform.cpp
		platform/screen/platform.cpp
		$<$<PLATFORM_ID:Darwin>:
			platform/battery/mac.mm
			platform/events/mac.mm
			>
		$<$<PLATFORM_ID:Windows>:
			platform/battery/windows.cpp
			platform/events/windows.cpp
			platform/winapi/fakeqwidgetwinapi.cpp
			>
		$<$<PLATFORM_ID:FreeBSD>:
			platform/battery/freebsd.cpp
			platform/poweractions/freebsd.cpp
			platform/events/freebsd.cpp
			platform/screen/freedesktop.cpp
			>
		$<$<PLATFORM_ID:Linux>:
			platform/battery/upower.cpp
			platform/dbus/endpoints.cpp
			platform/events/dbus/dbusplatformbase.cpp
			platform/events/dbus/logind.cpp
			platform/events/dbus/consolekit.cpp
			platform/events/dbus/upower.cpp
			platform/poweractions/upower.cpp
			platform/screen/freedesktop.cpp
			>
	SETTINGS liznoosettings.xml
	QT_COMPONENTS Concurrent Widgets ${LIZNOO_DBUS_QT_DEP}
	LINK_LIBRARIES
		PkgConfig::Qwt
		$<$<PLATFORM_ID:Darwin>:"-framework IOKit -framework CoreFoundation">
	INSTALL_SHARE
	)
