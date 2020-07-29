cmake_minimum_required (VERSION 3.10)
project (leechcraft_tabslist)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS tabslist.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
