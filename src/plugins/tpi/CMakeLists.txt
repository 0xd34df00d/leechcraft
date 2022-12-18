cmake_minimum_required (VERSION 3.10)
project (tpi)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS tpi.cpp infomodelmanager.cpp
	QT_COMPONENTS Gui
	INSTALL_SHARE
	)
