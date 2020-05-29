cmake_minimum_required (VERSION 3.3)
project (leechcraft_deadlyrics)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		deadlyrics.cpp
		searcher.cpp
		lyricscache.cpp
		hascirylsearcher.cpp
	RESOURCES deadlyricsresources.qrc
	QT_COMPONENTS Gui Network Xml
	INSTALL_SHARE
	)
