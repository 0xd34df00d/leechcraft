cmake_minimum_required (VERSION 3.3)
project (leechcraft_glance)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		core.cpp
		glance.cpp
		glanceitem.cpp
		glanceshower.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
