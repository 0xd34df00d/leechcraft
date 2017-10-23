cmake_minimum_required (VERSION 3.3)
project (leechcraft_hotsensors)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	hotsensors.cpp
	historymanager.cpp
	plotmanager.cpp
	sensorsgraphmodel.cpp
	contextwrapper.cpp
	backend.cpp
	)

set (ADDITIONAL_LIBS)

if (APPLE)
	set (SRCS ${SRCS} macosbackend.cpp)
	set (ADDITIONAL_LIBS "-framework IOKit;-framework CoreFoundation")
elseif (UNIX)
	find_package (Sensors)

	include_directories (${SENSORS_INCLUDE_DIR})
	set (ADDITIONAL_LIBS ${SENSORS_LIBRARIES})

	set (SRCS ${SRCS} lmsensorsbackend.cpp)
else ()
	message (STATUS "HotSensors: unsupported platform")
endif ()

CreateTrs("hotsensors" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("hotsensors" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_hotsensors SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_hotsensors
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
install (TARGETS leechcraft_hotsensors DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/hotsensors DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_hotsensors QuickWidgets)
