# Find the Qwt 5.x includes and library, either the version linked to Qt3 or the version linked to Qt4
#
# On Windows it makes these assumptions:
#    - the Qwt DLL is where the other DLLs for Qt are (QT_DIR\bin) or in the path
#    - the Qwt .h files are in QT_DIR\include\Qwt or in the path
#    - the Qwt .lib is where the other LIBs for Qt are (QT_DIR\lib) or in the path
#
# Qwt6_INCLUDE_DIR - where to find qwt.h if Qwt
# Qwt6_Qt4_LIBRARY - The Qwt6 library linked against Qt4 (if it exists)
# Qwt6_Qt3_LIBRARY - The Qwt6 library linked against Qt4 (if it exists)
# Qwt6_Qt4_FOUND   - Qwt6 was found and uses Qt4
# Qwt6_Qt3_FOUND   - Qwt6 was found and uses Qt3
# Qwt6_FOUND - Set to TRUE if Qwt6 was found (linked either to Qt3 or Qt4)

# Copyright (c) 2007, Pau Garcia i Quiles, <pgquiles@elpauer.org>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Condition is "(A OR B) AND C", CMake does not support parentheses but it evaluates left to right
IF(Qwt6_Qt4_LIBRARY OR Qwt6_Qt3_LIBRARY AND Qwt6_INCLUDE_DIR)
    SET(Qwt6_FIND_QUIETLY TRUE)
ENDIF(Qwt6_Qt4_LIBRARY OR Qwt6_Qt3_LIBRARY AND Qwt6_INCLUDE_DIR)

IF(NOT QT4_FOUND)
	FIND_PACKAGE( Qt4 REQUIRED QUIET )
ENDIF(NOT QT4_FOUND)

IF( QT4_FOUND )
	# Is Qwt6 installed? Look for header files
	FIND_PATH( Qwt6_INCLUDE_DIR qwt.h 
               PATHS ${QT_INCLUDE_DIR} /usr/local/qwt6/include /usr/include/qwt6
               PATH_SUFFIXES qwt qwt6 qwt-qt4 qwt6-qt4 qwt-qt3 qwt6-qt3 include qwt/include qwt6/include qwt-qt4/include qwt6-qt4/include qwt-qt3/include qwt6-qt3/include ENV PATH)
	
	# Find Qwt version
	IF( Qwt6_INCLUDE_DIR )
		FILE( READ ${Qwt6_INCLUDE_DIR}/qwt_global.h QWT_GLOBAL_H )
		STRING( REGEX MATCH "#define *QWT_VERSION *(0x05*)" QWT_IS_VERSION_5 ${QWT_GLOBAL_H})
		
		IF( QWT_IS_VERSION_5 )
		STRING(REGEX REPLACE ".*#define[\\t\\ ]+QWT_VERSION_STR[\\t\\ ]+\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*" "\\1" Qwt_VERSION "${QWT_GLOBAL_H}")

		# Find Qwt6 library linked to Qt4
		FIND_LIBRARY( Qwt6_Qt4_TENTATIVE_LIBRARY NAMES qwt6-qt4 qwt-qt4 qwt6 qwt PATHS /usr/local/qwt/lib /usr/local/lib /usr/lib )
		IF( UNIX AND NOT CYGWIN)
			IF( Qwt6_Qt4_TENTATIVE_LIBRARY )
				#MESSAGE("Qwt6_Qt4_TENTATIVE_LIBRARY = ${Qwt6_Qt4_TENTATIVE_LIBRARY}")
				EXECUTE_PROCESS( COMMAND "ldd" ${Qwt6_Qt4_TENTATIVE_LIBRARY} OUTPUT_VARIABLE Qwt_Qt4_LIBRARIES_LINKED_TO )
				STRING( REGEX MATCH "QtCore" Qwt6_IS_LINKED_TO_Qt4 ${Qwt_Qt4_LIBRARIES_LINKED_TO})
				IF( Qwt6_IS_LINKED_TO_Qt4 )
					SET( Qwt6_Qt4_LIBRARY ${Qwt6_Qt4_TENTATIVE_LIBRARY} )
					SET( Qwt6_Qt4_FOUND TRUE )
					IF (NOT Qwt6_FIND_QUIETLY)
						MESSAGE( STATUS "Found Qwt: ${Qwt6_Qt4_LIBRARY}" )
					ENDIF (NOT Qwt6_FIND_QUIETLY)
				ENDIF( Qwt6_IS_LINKED_TO_Qt4 )
			ENDIF( Qwt6_Qt4_TENTATIVE_LIBRARY )
		ELSE( UNIX AND NOT CYGWIN)
		# Assumes qwt.dll is in the Qt dir
			SET( Qwt6_Qt4_LIBRARY ${Qwt6_Qt4_TENTATIVE_LIBRARY} )
			SET( Qwt6_Qt4_FOUND TRUE )
			IF (NOT Qwt6_FIND_QUIETLY)
				MESSAGE( STATUS "Found Qwt version ${Qwt_VERSION} linked to Qt4" )
			ENDIF (NOT Qwt6_FIND_QUIETLY)
		ENDIF( UNIX AND NOT CYGWIN)
		
		
		# Find Qwt6 library linked to Qt3
		FIND_LIBRARY( Qwt6_Qt3_TENTATIVE_LIBRARY NAMES qwt-qt3 qwt qwt6-qt3 qwt6 )
		IF( UNIX AND NOT CYGWIN)
			IF( Qwt6_Qt3_TENTATIVE_LIBRARY )
				#MESSAGE("Qwt6_Qt3_TENTATIVE_LIBRARY = ${Qwt6_Qt3_TENTATIVE_LIBRARY}")
				EXECUTE_PROCESS( COMMAND "ldd" ${Qwt6_Qt3_TENTATIVE_LIBRARY} OUTPUT_VARIABLE Qwt-Qt3_LIBRARIES_LINKED_TO )
				STRING( REGEX MATCH "qt-mt" Qwt6_IS_LINKED_TO_Qt3 ${Qwt-Qt3_LIBRARIES_LINKED_TO})
				IF( Qwt6_IS_LINKED_TO_Qt3 )
					SET( Qwt6_Qt3_LIBRARY ${Qwt6_Qt3_TENTATIVE_LIBRARY} )
					SET( Qwt6_Qt3_FOUND TRUE )
					IF (NOT Qwt6_FIND_QUIETLY)
						MESSAGE( STATUS "Found Qwt version ${Qwt_VERSION} linked to Qt3" )
					ENDIF (NOT Qwt6_FIND_QUIETLY)
				ENDIF( Qwt6_IS_LINKED_TO_Qt3 )
			ENDIF( Qwt6_Qt3_TENTATIVE_LIBRARY )
		ELSE( UNIX AND NOT CYGWIN)
			SET( Qwt6_Qt3_LIBRARY ${Qwt6_Qt3_TENTATIVE_LIBRARY} )
			SET( Qwt6_Qt3_FOUND TRUE )
			IF (NOT Qwt6_FIND_QUIETLY)
				MESSAGE( STATUS "Found Qwt: ${Qwt6_Qt3_LIBRARY}" )
			ENDIF (NOT Qwt6_FIND_QUIETLY)
		ENDIF( UNIX AND NOT CYGWIN)
		
		ENDIF( QWT_IS_VERSION_5 )
		
		IF( Qwt6_Qt4_FOUND OR Qwt6_Qt3_FOUND )
			SET( Qwt6_FOUND TRUE )
		ENDIF( Qwt6_Qt4_FOUND OR Qwt6_Qt3_FOUND )
		
		MARK_AS_ADVANCED( Qwt6_INCLUDE_DIR Qwt6_Qt4_LIBRARY Qwt6_Qt3_LIBRARY )
	ENDIF( Qwt6_INCLUDE_DIR )

   	IF (NOT Qwt6_FOUND AND Qwt6_FIND_REQUIRED)
      		MESSAGE(FATAL_ERROR "Could not find Qwt 5.x")
   	ENDIF (NOT Qwt6_FOUND AND Qwt6_FIND_REQUIRED)

ENDIF( QT4_FOUND )
