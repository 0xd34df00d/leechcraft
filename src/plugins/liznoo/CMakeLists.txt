cmake_minimum_required (VERSION 3.10)
project (liznoo)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_search_module (Qwt REQUIRED IMPORTED_TARGET Qt5Qwt Qt5Qwt6)

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
		xmlsettingsmanager.cpp
		platform/battery/batteryplatform.cpp
		platform/events/platformlayer.cpp
		platform/poweractions/platform.cpp
		platform/screen/screenplatform.cpp
		$<$<PLATFORM_ID:Darwin>:
			platform/battery/macplatform.mm
			platform/events/platformmac.mm
			>
		$<$<PLATFORM_ID:Windows>:
			platform/battery/winapiplatform.cpp
			platform/events/platformwinapi.cpp
			platform/winapi/fakeqwidgetwinapi.cpp
			>
		$<$<PLATFORM_ID:FreeBSD>:
			platform/battery/freebsdplatform.cpp
			platform/poweractions/freebsd.cpp
			platform/events/platformfreebsd.cpp
			platform/screen/freedesktop.cpp
			>
		$<$<PLATFORM_ID:Linux>:
			platform/common/dbusthread.cpp
			platform/common/connectorbase.cpp
			platform/poweractions/pmutils.cpp
			platform/logind/logindconnector.cpp
			platform/battery/upowerplatform.cpp
			platform/upower/upowerconnector.cpp
			platform/poweractions/upower.cpp
			platform/consolekit/connector.cpp
			platform/screen/freedesktop.cpp
			>
	SETTINGS liznoosettings.xml
	QT_COMPONENTS Concurrent Widgets ${LIZNOO_DBUS_QT_DEP}
	LINK_LIBRARIES
		PkgConfig::Qwt
		$<$<PLATFORM_ID:Darwin>:"-framework IOKit -framework CoreFoundation">
	INSTALL_SHARE
	)
