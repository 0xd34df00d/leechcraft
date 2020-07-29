cmake_minimum_required (VERSION 3.10)
project (leechcraft_lmp_jos)
include (InitLCPlugin NO_POLICY_SCOPE)

pkg_check_modules (LibGPod REQUIRED IMPORTED_TARGET libgpod-1.0)
pkg_check_modules (LibIMobileDevice REQUIRED IMPORTED_TARGET libimobiledevice-1.0)

LC_DEFINE_PLUGIN (
	SRCS
		jos.cpp
		mobileraii.cpp
		devmanager.cpp
		connection.cpp
		uploadmanager.cpp
		afcfile.cpp
		gpoddb.cpp
	QT_COMPONENTS Concurrent Gui
	LINK_LIBRARIES PkgConfig::LibGPod PkgConfig::LibIMobileDevice
	)
