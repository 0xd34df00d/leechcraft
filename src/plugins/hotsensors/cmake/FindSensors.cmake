FIND_PATH(SENSORS_INCLUDE_DIR sensors/sensors.h)
FIND_LIBRARY(SENSORS_LIBRARIES NAMES sensors)

if(SENSORS_LIBRARIES AND SENSORS_INCLUDE_DIR)
	set(SENSORS_FOUND TRUE)
	message(STATUS "Found lm_sensors library: ${SENSORS_LIBRARIES} and include dir: ${SENSORS_INCLUDE_DIR}")
else ()
	message(FATAL_ERROR "Could NOT find lm_sensors")
endif()

mark_as_advanced(SENSORS_INCLUDE_DIR SENSORS_LIBRARIES)
