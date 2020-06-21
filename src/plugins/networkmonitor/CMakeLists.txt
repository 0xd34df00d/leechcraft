cmake_minimum_required (VERSION 3.3)
project (leechcraft_networkmonitor)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		networkmonitor.cpp
		requestmodel.cpp
		headermodel.cpp
	QT_COMPONENTS Network Widgets
	INSTALL_SHARE
	)
