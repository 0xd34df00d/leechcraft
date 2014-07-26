find_package (PkgConfig)
pkg_check_modules (PC_VMIME vmime)
find_path (VMIME_INCLUDE_DIR vmime/vmime.hpp
	HINTS
		${PC_VMIME_INCLUDEDIR} ${PC_VMIME_INCLUDE_DIRS}
	PATH_SUFFIXES
		vmime)
find_library (VMIME_LIBRARIES
	NAMES vmime
	HINTS
		${PC_VMIME_LIBDIR} ${PC_VMIME_LIBRARY_DIRS}
		)

if (VMIME_INCLUDE_DIR AND VMIME_LIBRARIES)
	set (VMIME_FOUND TRUE)
endif ()

if (VMIME_FOUND)
	message (STATUS "Found vmime libraries at ${VMIME_LIBRARIES}")
	message (STATUS "Found vmime headers at ${VMIME_INCLUDE_DIR}")
else ()
	if (VMime_FIND_REQUIRED)
		message (FATAL_ERROR "Could NOT find required vmime library, aborting")
	else ()
		message (STATUS "Could NOT find vmime")
	endif ()
endif ()
