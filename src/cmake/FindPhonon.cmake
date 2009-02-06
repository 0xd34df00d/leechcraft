# Find libphonon
# Once done this will define
#
#  QT_PHONON_FOUND    - system has Phonon Library
#  QT_PHONON_INCLUDES - the Phonon include directory
#  QT_PHONON_LIBS     - link these to use Phonon

# Copyright (c) 2008, Matthias Kretz <kretz@kde.org>
# Copyright (c) 2008, Voker57 <voker57@gmail.com>
# Copyright (c) 2009, 0xd34df00d <0xd34df00d@gmail.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# 0xd34df00d, 11 Jan 2009
# We should search for Qt's Phonon, which is all in lowercase etc.

if(QT_PHONON_FOUND)
   # Already found, nothing more to do
else(QT_PHONON_FOUND)
   if(QT_PHONON_INCLUDE_DIR AND QT_PHONON_LIBRARY)
      set(QT_PHONON_FIND_QUIETLY TRUE)
   endif(QT_PHONON_INCLUDE_DIR AND QT_PHONON_LIBRARY)

   # As discussed on kde-buildsystem: first look at CMAKE_PREFIX_PATH, then at the suggested PATHS (kde4 install dir)
   find_library(QT_PHONON_LIBRARY NAMES phonon PATHS ${KDE4_LIB_INSTALL_DIR} ${QT_LIBRARY_DIR} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
   # then at the default system locations (CMAKE_SYSTEM_PREFIX_PATH, i.e. /usr etc.)
   find_library(QT_PHONON_LIBRARY NAMES phonon)

   find_path(QT_PHONON_INCLUDE_DIR NAMES phonon/phonon PATHS ${QT_INCLUDE_DIR} ${INCLUDE_INSTALL_DIR} NO_SYSTEM_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)

   if(QT_PHONON_INCLUDE_DIR AND QT_PHONON_LIBRARY)
      set(QT_PHONON_LIBS ${phonon_LIB_DEPENDS} ${QT_PHONON_LIBRARY})
      set(QT_PHONON_INCLUDES ${QT_PHONON_INCLUDE_DIR}/phonon)
      set(QT_PHONON_FOUND TRUE)
   else(QT_PHONON_INCLUDE_DIR AND QT_PHONON_LIBRARY)
      set(QT_PHONON_FOUND FALSE)
   endif(QT_PHONON_INCLUDE_DIR AND QT_PHONON_LIBRARY)

   if(QT_PHONON_FOUND)
      if(NOT QT_PHONON_FIND_QUIETLY)
         message(STATUS "Found Phonon: ${QT_PHONON_LIBRARY}")
         message(STATUS "Found Phonon Includes: ${QT_PHONON_INCLUDES}")
      endif(NOT QT_PHONON_FIND_QUIETLY)
   else(QT_PHONON_FOUND)
      if(Phonon_FIND_REQUIRED)
         if(NOT QT_PHONON_INCLUDE_DIR)
            message(STATUS "Phonon includes NOT found!")
         endif(NOT QT_PHONON_INCLUDE_DIR)
         if(NOT QT_PHONON_LIBRARY)
            message(STATUS "Phonon library NOT found!")
         endif(NOT QT_PHONON_LIBRARY)
         message(FATAL_ERROR "Phonon library or includes NOT found!")
      else(Phonon_FIND_REQUIRED)
         message(STATUS "Unable to find Phonon")
      endif(Phonon_FIND_REQUIRED)
   endif(QT_PHONON_FOUND)


   mark_as_advanced(QT_PHONON_INCLUDE_DIR QT_PHONON_LIBRARY QT_PHONON_INCLUDES)
endif(QT_PHONON_FOUND)
