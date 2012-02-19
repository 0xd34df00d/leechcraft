# Try to find the Qt4 binding of the Telepathy library
# TELEPATHY_QT4_FOUND - system has TelepathyQt4
# TELEPATHY_QT4_INCLUDE_DIR - the TelepathyQt4 include directory
# TELEPATHY_QT4_LIBRARIES - Link these to use TelepathyQt4

# Copyright (c) 2008, Allen Winter <winter@kde.org>
# Copyright (c) 2009, Andre Moreira Magalhaes <andrunko@gmail.com>
# Copyright (c) 2010, Casper van Donderen <casper.vandonderen@basyskom.de>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

set(TELEPATHY_QT4_FIND_REQUIRED ${TelepathyQt4_FIND_REQUIRED})
if(TELEPATHY_QT4_INCLUDE_DIR AND TELEPATHY_QT4_LIBRARIES)
  # Already in cache, be silent
  set(TELEPATHY_QT4_FIND_QUIETLY TRUE)
endif(TELEPATHY_QT4_INCLUDE_DIR AND TELEPATHY_QT4_LIBRARIES)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_TELEPATHY_QT4 QUIET TelepathyQt4>=0.2.0)
endif(PKG_CONFIG_FOUND)

find_path(TELEPATHY_QT4_INCLUDE_DIR
          NAMES Types
          HINTS
          ${PC_TELEPATHY_QT4_INCLUDEDIR}
          ${PC_TELEPATHY_QT4_INCLUDE_DIRS}
          PATH_SUFFIXES TelepathyQt TelepathyQt4
)

find_library(TELEPATHY_QT4_LIBRARIES
             NAMES telepathy-qt4
             HINTS
             ${PC_TELEPATHY_QT4_LIBDIR}
             ${PC_TELEPATHY_QT4_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TelepathyQt4 DEFAULT_MSG
                                  TELEPATHY_QT4_LIBRARIES TELEPATHY_QT4_INCLUDE_DIR)

