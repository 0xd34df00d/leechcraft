CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_hotstreams)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (QJSON REQUIRED)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QJSON_INCLUDE_DIR}
	)
SET (SRCS
	hotstreams.cpp
	icecastfetcher.cpp
	radiostation.cpp
	rockradiolistfetcher.cpp
	somafmlistfetcher.cpp
	stealkilllistfetcher.cpp
	streamlistfetcherbase.cpp
	stringlistradiostation.cpp
	)

IF (QJSON_FOUND)
	ADD_DEFINITIONS (-DHAVE_QJSON)
	SET (SRCS ${SRCS} audioaddictstreamfetcher.cpp)
ENDIF (QJSON_FOUND)

SET (RESOURCES hotstreamsresources.qrc)
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_hotstreams SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_hotstreams
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QJSON_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_hotstreams DESTINATION ${LC_PLUGINS_DEST})
