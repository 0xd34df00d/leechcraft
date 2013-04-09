CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_hotsensors)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

FIND_PACKAGE (Qwt REQUIRED)

SET (QT_USE_QTSVG TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)
SET (SRCS
	hotsensors.cpp
	sensorsmanager.cpp
	historymanager.cpp
	plotmanager.cpp
	)
#CreateTrs("hotsensors" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("hotsensors" "en;ru_RU" "${SRCS}" "${FORMS}" "")

ADD_LIBRARY (leechcraft_hotsensors SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_hotsensors
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QWT_LIBRARIES}
	sensors
	)
INSTALL (TARGETS leechcraft_hotsensors DESTINATION ${LC_PLUGINS_DEST})
