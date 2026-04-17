cmake_minimum_required (VERSION 3.10)
project (vrooby)
include (InitLCPlugin NO_POLICY_SCOPE)

if (NOT APPLE AND NOT WIN32)
	set (VROOBY_DBUS_QT_DEP DBus)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		$<$<NOT:$<PLATFORM_ID:Darwin,Windows>>:
			backends/udisks2/udisks2backend.cpp
			backends/udisks2/dbus/manager.cpp
			>
		trayproxymodel.cpp
		trayview.cpp
		vrooby.cpp
	QT_COMPONENTS ${VROOBY_DBUS_QT_DEP} QuickWidgets
	INSTALL_SHARE
	)
