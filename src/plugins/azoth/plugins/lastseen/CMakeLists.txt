cmake_minimum_required (VERSION 3.10)
project (azoth_lastseen)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		lastseen.cpp
		ondiskstorage.cpp
	QT_COMPONENTS Concurrent Sql Widgets
	INSTALL_SHARE
	)
