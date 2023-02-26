cmake_minimum_required (VERSION 3.10)
project (glance)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		glance.cpp
		glanceview.cpp
	QT_COMPONENTS QuickWidgets
	INSTALL_SHARE
	)
