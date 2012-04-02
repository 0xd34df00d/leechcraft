CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_nativeemoticons)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

SET (NATIVEEMOTICONS_SRCS
	nativeemoticons.cpp
	nativeemoticonssource.cpp
	kopeteemoticonssource.cpp
	psiplusemoticonssource.cpp
	baseemoticonssource.cpp
	)
SET (NATIVEEMOTICONS_HEADERS
	nativeemoticons.h
	nativeemoticonssource.h
	kopeteemoticonssource.h
	psiplusemoticonssource.h
	baseemoticonssource.h
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (NATIVEEMOTICONS_MOC_SRCS ${NATIVEEMOTICONS_HEADERS})
ENDIF (NOT LC_NO_MOC)

ADD_LIBRARY (leechcraft_azoth_nativeemoticons SHARED
	${NATIVEEMOTICONS_SRCS}
	${NATIVEEMOTICONS_MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_nativeemoticons
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_azoth_nativeemoticons DESTINATION ${LC_PLUGINS_DEST})
INSTALL (DIRECTORY share/azoth DESTINATION share/leechcraft)
