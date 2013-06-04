# cmake macro to test LibOTR

# Copyright (c) 2008, Michael Zanetti <michael_zanetti @ gnx.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroEnsureVersion)
include(FindPackageHandleStandardArgs)

if (LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY)
    # Already in cache, be silent
    set(LIBOTR_FIND_QUIETLY TRUE)
endif (LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY)

find_path(LIBOTR_INCLUDE_DIR libotr/version.h)

find_library(LIBOTR_LIBRARY NAMES otr libotr)

# Determine version information from libotr/version.h
if( LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY )
  execute_process(COMMAND grep "OTRL_VERSION" "${LIBOTR_INCLUDE_DIR}/libotr/version.h" OUTPUT_VARIABLE output)
  string(REGEX MATCH "OTRL_VERSION \"[0-9]+\\.[0-9]+\\.[0-9]+" LIBOTR_VERSION "${output}")
  string(REGEX REPLACE "^OTRL_VERSION \"" "" LIBOTR_VERSION "${LIBOTR_VERSION}")
  # Check if version is at least 3.2.0
  MACRO_ENSURE_VERSION("3.2.0" ${LIBOTR_VERSION} LIBOTR_FOUND)

  if( LIBOTR_FOUND )
    if( NOT LIBOTR_FIND_QUIETLY )
      message( STATUS "Found libotr: ${LIBOTR_LIBRARY}")
    endif( NOT LIBOTR_FIND_QUIETLY )
  else( LIBOTR_FOUND )
    message(STATUS "libotr version 3.2.0 or greater required but found ${LIBOTR_VERSION}.")
  endif( LIBOTR_FOUND )

endif( LIBOTR_INCLUDE_DIR AND LIBOTR_LIBRARY )
