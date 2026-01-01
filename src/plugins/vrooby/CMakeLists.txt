cmake_minimum_required (VERSION 3.10)
project (vrooby)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		backends/udisks2/udisks2backend.cpp
		backends/udisks2/dbus/manager.cpp
		trayproxymodel.cpp
		trayview.cpp
		vrooby.cpp
	QT_COMPONENTS DBus QuickWidgets
	INSTALL_SHARE
	)
