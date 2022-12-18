cmake_minimum_required (VERSION 3.10)
project (lastfmscrobble)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (LastFM REQUIRED)

LC_DEFINE_PLUGIN (
	SRCS
		lastfmsubmitter.cpp
		xmlsettingsmanager.cpp
		lastfmscrobble.cpp
		pendingsimilarartists.cpp
		albumartfetcher.cpp
		authenticator.cpp
		basesimilarartists.cpp
		pendingrecommendedartists.cpp
		util.cpp
		lastfmradiostation.cpp
		lastfmradiotuner.cpp
		recentreleasesfetcher.cpp
		pendingartistbio.cpp
		receventsfetcher.cpp
		eventsfetchaggregator.cpp
		eventattendmarker.cpp
		hypedartistsfetcher.cpp
		hypedtracksfetcher.cpp
		imagesfetcher.cpp
	RESOURCES lastfmscrobbleresources.qrc
	SETTINGS lastfmscrobblesettings.xml
	QT_COMPONENTS Gui Network Xml
	LINK_LIBRARIES LastFM::LastFM
	)
