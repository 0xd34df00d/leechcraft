# Find HUpnp - library for building UPnP devices and control points conforming to the UPnP Device Architecture version 1.1
#
# This module defines
#  HUpnp_FOUND - whether the qsjon library was found
#  HUpnp_LIBRARIES - the qjson library
#  HUpnp_INCLUDE_DIR - the include path of the qjson library
#

if (HUpnp_INCLUDE_DIR AND HUpnp_LIBRARIES)
	# Already in cache
	set (HUpnp_FOUND TRUE)
else ()
	if (NOT WIN32)
		find_library (HUpnp_LIBRARY
			NAMES
				HUpnp
			PATHS
				ENV
		)
		find_library (HUpnpAv_LIBRARY
			NAMES
				HUpnpAv
			PATHS
				ENV
		)
	else ()
		if (NOT DEFINED HUpnp_DIR)
			if (HUpnp_FIND_REQUIRED)
				message(FATAL_ERROR "Please set HUpnp_DIR variable")
			else ()
				message(STATUS "Please set HUpnp_DIR variable for DLNiwe")
			endif ()
		endif ()
	
		if (MINGW)
			find_library (HUpnp_LIBRARIES NAMES libHUpnp1.a PATHS ${HUpnp_DIR}/lib)
		endif ()
	endif ()

	find_path (HUpnp_INCLUDE_DIR
		NAMES
			HUpnpCore/HUpnp
		PATHS
			${HUpnp_DIR}/include
			${INCLUDE_INSTALL_DIR}
	)

	if (HUpnp_INCLUDE_DIR AND HUpnp_LIBRARY AND HUpnpAv_LIBRARY)
		set (HUpnp_FOUND 1)
	endif ()

	if (HUpnp_FOUND)
		set (HUpnp_LIBRARIES ${HUpnp_LIBRARY} ${HUpnpAv_LIBRARY})
		message (STATUS "Found the HUpnp libraries at ${HUpnp_LIBRARIES}")
		message (STATUS "Found the HUpnp headers at ${HUpnp_INCLUDE_DIR}")
		if (WIN32)
			message (STATUS ${_WIN32_ADDITIONAL_MESS})
		endif ()
	else ()
		if (HUpnp_FIND_REQUIRED)
			message (FATAL_ERROR "Could NOT find required HUpnp library, aborting")
		else ()
			message (STATUS "Could NOT find HUpnp")
		endif ()
	endif ()

endif ()
