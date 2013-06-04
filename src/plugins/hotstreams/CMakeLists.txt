cmake_minimum_required (VERSION 2.8)
project (leechcraft_hotstreams)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (QJSON REQUIRED)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)
set (SRCS
	hotstreams.cpp
	icecastfetcher.cpp
	radiostation.cpp
	somafmlistfetcher.cpp
	stealkilllistfetcher.cpp
	streamlistfetcherbase.cpp
	stringlistradiostation.cpp
	)

if (QJSON_FOUND)
	add_definitions (-DHAVE_QJSON)
	set (SRCS
		${SRCS}
		audioaddictstreamfetcher.cpp
		rockradiolistfetcher.cpp)
endif ()

set (RESOURCES hotstreamsresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_hotstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
target_link_libraries (leechcraft_hotstreams
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
install (TARGETS leechcraft_hotstreams DESTINATION ${LC_PLUGINS_DEST})
