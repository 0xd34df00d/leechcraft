cmake_minimum_required (VERSION 3.10)
project (hotsensors)
include (InitLCPlugin NO_POLICY_SCOPE)

if (UNIX AND NOT APPLE)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (Sensors)
	include_directories (${SENSORS_INCLUDE_DIR})
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		hotsensors.cpp
		historymanager.cpp
		plotmanager.cpp
		sensorsgraphmodel.cpp
		contextwrapper.cpp
		backend.cpp
		$<$<PLATFORM_ID:Linux>:lmsensorsbackend.cpp>
		$<$<PLATFORM_ID:Darwin>:macosbackend.cpp>
	QT_COMPONENTS QuickWidgets
	LINK_LIBRARIES
		$<$<PLATFORM_ID:Linux>:${SENSORS_LIBRARIES}>
		$<$<PLATFORM_ID:Darwin>:"-framework IOKit -framework CoreFoundation">
	INSTALL_SHARE
	)
