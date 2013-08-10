# Try to find the HTML Tidy lib
#
#  LIBTIDY_FOUND        - true if LIBTIDY was found
#  LIBTIDY_INCLUDE_DIRS - Directory to include to get LIBTIDY headers
#                         Note: always include LIBTIDY headers as e.g.,
#                         tidy/tidy.h
#  LIBTIDY_LIBRARIES    - Libraries to link against for the LIBTIDY
#

if (LIBTIDY_INCLUDE_DIR)
  set(LibTidy_FIND_QUIETLY TRUE)
endif (LIBTIDY_INCLUDE_DIR)

# Look for the header file.
find_path(LIBTIDY_INCLUDE_DIR tidy.h PATH_SUFFIXES tidy PATHS ${LIBTIDY_DIR}/include)
mark_as_advanced(LIBTIDY_INCLUDE_DIR)

# Look for the library.
find_library(LIBTIDY_LIBRARY NAMES tidy PATHS ${LIBTIDY_DIR}/lib)
mark_as_advanced(LIBTIDY_LIBRARY)

if (LIBTIDY_INCLUDE_DIR AND LIBTIDY_LIBRARY)
  set(LIBTIDY_FOUND 1)
  set(LIBTIDY_LIBRARIES ${LIBTIDY_LIBRARY})
  set(LIBTIDY_INCLUDE_DIRS ${LIBTIDY_INCLUDE_DIR})
  message("-- Found libtidy library -- " ${LIBTIDY_LIBRARY})
  message("-- Found libtidy include path -- " ${LIBTIDY_INCLUDE_DIR})
else (LIBTIDY_INCLUDE_DIR AND LIBTIDY_LIBRARY)
  set(LIBTIDY_FOUND 0)
  set(LIBTIDY_LIBRARIES)
  set(LIBTIDY_INCLUDE_DIRS)
endif (LIBTIDY_INCLUDE_DIR AND LIBTIDY_LIBRARY)

