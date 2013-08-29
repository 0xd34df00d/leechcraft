# - Try to find the Taglib library
# Once done this will define
#
#  TAGLIB_FOUND - system has the taglib library
#  TAGLIB_CFLAGS - the taglib cflags
#  TAGLIB_LIBRARIES - The libraries needed to use taglib

# Copyright (c) 2006, Laurent Montel, <montel@kde.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(NOT TAGLIB_MIN_VERSION)
  set(TAGLIB_MIN_VERSION "1.4")
endif()

if(NOT WIN32)
    find_program(TAGLIBCONFIG_EXECUTABLE NAMES taglib-config PATHS
       ${BIN_INSTALL_DIR}
    )
endif()

#reset vars
set(TAGLIB_LIBRARIES)
set(TAGLIB_CFLAGS)

# if taglib-config has been found
if(TAGLIBCONFIG_EXECUTABLE)

  exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --version RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_VERSION)

  if(TAGLIB_VERSION STRLESS "${TAGLIB_MIN_VERSION}")
     message(STATUS "TagLib version not found: version searched :${TAGLIB_MIN_VERSION}, found ${TAGLIB_VERSION}")
     set(TAGLIB_FOUND FALSE)
  else()

     exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --libs RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_LIBRARIES)

     exec_program(${TAGLIBCONFIG_EXECUTABLE} ARGS --cflags RETURN_VALUE _return_VALUE OUTPUT_VARIABLE TAGLIB_CFLAGS)

     if(TAGLIB_LIBRARIES AND TAGLIB_CFLAGS)
        set(TAGLIB_FOUND TRUE)
        message(STATUS "Found taglib: ${TAGLIB_LIBRARIES}")
     endif()
     string(REGEX REPLACE " *-I" ";" TAGLIB_INCLUDES "${TAGLIB_CFLAGS}")
  endif()
  mark_as_advanced(TAGLIB_CFLAGS TAGLIB_LIBRARIES TAGLIB_INCLUDES)

else()

  include(FindPackageHandleStandardArgs)

  find_path(TAGLIB_INCLUDES
    NAMES
        taglib/tag.h
    PATHS
    ${KDE4_INCLUDE_DIR}
    ${INCLUDE_INSTALL_DIR}
    ${TAGLIB_DIR}/include)

  find_library(TAGLIB_LIBRARIES
    NAMES tag
    PATHS
    ${KDE4_LIB_DIR}
    ${LIB_INSTALL_DIR}
    ${TAGLIB_DIR}/lib)

  find_package_handle_standard_args(Taglib DEFAULT_MSG
                                    TAGLIB_INCLUDES TAGLIB_LIBRARIES)
endif()

if(TAGLIB_FOUND)
  if(NOT Taglib_FIND_QUIETLY AND TAGLIBCONFIG_EXECUTABLE)
    message(STATUS "Taglib found: ${TAGLIB_LIBRARIES}")
  endif()
else()
  if(Taglib_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find Taglib")
  endif()
endif()

