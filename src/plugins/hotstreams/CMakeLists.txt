cmake_minimum_required (VERSION 2.8)
project (leechcraft_hotstreams)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
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
	)

set (RESOURCES hotstreamsresources.qrc)
QtAddResources (RCCS ${RESOURCES})

CreateTrs("hotstreams" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("hotstreams" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_hotstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_hotstreams
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_hotstreams DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_hotstreams Concurrent Gui Network Xml)
