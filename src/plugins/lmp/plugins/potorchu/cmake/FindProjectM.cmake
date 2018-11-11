find_package (PkgConfig)
pkg_check_modules (PC_PROJECTM QUIET libprojectM)

# Include dir
find_path (ProjectM_INCLUDE_DIR
	NAMES libprojectM/projectM.hpp
	PATHS ${PC_PROJECTM_INCLUDE_DIRS}
	)

# Finally the library itself
find_library(ProjectM_LIBRARIES
	NAMES projectM
	PATHS ${PC_PROJECTM_LIBRARY_DIRS}
	)

set (ProjectM_VERSION ${PC_PROJECTM_VERSION})

if (ProjectM_INCLUDE_DIR AND ProjectM_LIBRARIES)
	message (STATUS "Found ProjectM version: ${ProjectM_VERSION}")
	message (STATUS "Found ProjectM includes at: ${ProjectM_INCLUDE_DIR}")
	message (STATUS "Found ProjectM libraries at: ${ProjectM_LIBRARIES}")
else ()
	message (FATAL_ERROR "Could NOT find ProjectM")
endif ()
