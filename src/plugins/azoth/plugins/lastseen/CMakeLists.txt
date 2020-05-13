cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_lastseen)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (azoth_lastseen
	SRCS
		lastseen.cpp
		ondiskstorage.cpp
	QT_COMPONENTS Concurrent Sql Widgets
	TS_LANGUAGES en es ru_RU uk_UA
	INSTALL_SHARE
	)
