cmake_minimum_required (VERSION 3.10)
project (auscrie)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		auscrie.cpp
		platform.cpp
		shooterdialog.cpp
		util.cpp
		$<$<PLATFORM_ID:Linux,FreeBSD>:platform_x11.cpp>
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
