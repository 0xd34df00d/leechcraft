# - Find libmaxminddb
# Find the native maxminddb includes and library
#
#  MMDB_INCLUDE_DIRS
#  MMDB_LIBRARIES
#  MMDB_FOUND

find_package(PkgConfig QUIET)
pkg_search_module(MMDB libmaxminddb)

if (NOT MMDB_FOUND)
	message (STATUS "Unable to find libmaxminddb via pkgconfig")
endif ()