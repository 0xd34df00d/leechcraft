cmake_minimum_required (VERSION 3.10)
project (leechcraft_hotstreams)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		audioaddictstreamfetcher.cpp
		hotstreams.cpp
		icecastfetcher.cpp
		icecastmodel.cpp
		radiostation.cpp
		rockradiolistfetcher.cpp
		somafmlistfetcher.cpp
		stealkilllistfetcher.cpp
		streamlistfetcherbase.cpp
		stringlistradiostation.cpp
	QT_COMPONENTS Concurrent Gui Network Xml
	INSTALL_SHARE
	)
