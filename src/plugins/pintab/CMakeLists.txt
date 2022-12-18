cmake_minimum_required (VERSION 3.10)
project (pintab)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS pintab.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
