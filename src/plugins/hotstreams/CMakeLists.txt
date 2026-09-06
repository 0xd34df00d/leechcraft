cmake_minimum_required (VERSION 3.10)
project (hotstreams)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		audioaddictstreamfetcher.cpp
		hotstreams.cpp
		icecastfetcher.cpp
		icecastmodel.cpp
		radiostation.cpp
		somafmlistfetcher.cpp
		stealkilllistfetcher.cpp
		streamlistfetcherbase.cpp
		stringlistradiostation.cpp
	RESOURCES hotstreamsresources.qrc
	QT_COMPONENTS Concurrent Gui Network Xml
	INSTALL_SHARE
	)
