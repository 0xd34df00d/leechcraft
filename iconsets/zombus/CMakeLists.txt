CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (leechcraft-icons-zombus)

INSTALL (DIRECTORY icons/ DESTINATION share/icons/zombus
	PATTERN ".svn" EXCLUDE)
IF (UNIX)
	INSTALL (FILES zombus DESTINATION /etc/leechcraft/themes)
ENDIF (UNIX)
