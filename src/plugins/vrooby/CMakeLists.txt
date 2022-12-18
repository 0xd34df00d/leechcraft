cmake_minimum_required (VERSION 3.10)
project (vrooby)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_VROOBY_UDISKS "Enable UDisks backend for Vrooby" ON)
option (ENABLE_VROOBY_UDISKS2 "Enable UDisks2 backend for Vrooby" ON)

add_compile_definitions (
	$<$<BOOL:${ENABLE_VROOBY_UDISKS}>:ENABLE_UDISKS>
	$<$<BOOL:${ENABLE_VROOBY_UDISKS2}>:ENABLE_UDISKS2>
	)

LC_DEFINE_PLUGIN (
	SRCS
		vrooby.cpp
		devbackend.cpp
		trayview.cpp
		flatmountableitems.cpp
		$<$<BOOL:${ENABLE_VROOBY_UDISKS}>:
			backends/udisks/udisksbackend.cpp
			>
		$<$<BOOL:${ENABLE_VROOBY_UDISKS2}>:
			backends/udisks2/udisks2backend.cpp
			backends/udisks2/dbus/manager.cpp
			>
	QT_COMPONENTS DBus QuickWidgets
	INSTALL_SHARE
	)
