cmake_minimum_required (VERSION 2.8)
project (leechcraft_lastfmscrobble)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package (LastFM REQUIRED)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LastFM_INCLUDE_DIR}
	)

set (SRCS
	lastfmsubmitter.cpp
	xmlsettingsmanager.cpp
	lastfmscrobble.cpp
	pendingsimilarartists.cpp
	albumartfetcher.cpp
	authenticator.cpp
	basesimilarartists.cpp
	pendingrecommendedartists.cpp
	util.cpp
	radiostation.cpp
	radiotuner.cpp
	recentreleasesfetcher.cpp
	pendingartistbio.cpp
	receventsfetcher.cpp
	eventsfetchaggregator.cpp
	eventattendmarker.cpp
	hypedartistsfetcher.cpp
	hypedtracksfetcher.cpp
	)
set (HEADERS
	lastfmsubmitter.h
	xmlsettingsmanager.h
	lastfmscrobble.h
	pendingsimilarartists.h
	albumartfetcher.h
	authenticator.h
	basesimilarartists.h
	pendingrecommendedartists.h
	util.h
	radiostation.h
	radiotuner.h
	recentreleasesfetcher.h
	pendingartistbio.h
	receventsfetcher.h
	eventsfetchaggregator.h
	eventattendmarker.h
	hypedartistsfetcher.h
	hypedtracksfetcher.h
	)
set (RESOURCES lastfmscrobbleresources.qrc)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
CreateTrs ("lastfmscrobble" "en;ru_RU" COMPILED_TRANSLATIONS)

add_library (leechcraft_lastfmscrobble SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_lastfmscrobble
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${LastFM_LIBRARIES}
	)

install (TARGETS leechcraft_lastfmscrobble RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES lastfmscrobblesettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
