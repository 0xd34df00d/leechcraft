cmake_minimum_required (VERSION 3.10)
project (auscrie)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		auscrie.cpp
		shooter.cpp
		shooterdialog.cpp
		util.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
