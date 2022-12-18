cmake_minimum_required (VERSION 3.10)
project (lads)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS lads.cpp
	QT_COMPONENTS DBus
	INSTALL_SHARED
	)
