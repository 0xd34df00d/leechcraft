CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
IF (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (lcnp)
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/GX")
	SET (CMAKE_EXE_LINKER "/entry:mainCRTStartup")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "-fvisibility=hidden -Wall -Wextra")
ENDIF (UNIX)

SET (QT_USE_QTDBUS TRUE)
FIND_PACKAGE (Qt4 REQUIRED)

INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR}
	)

SET (SRCS
	qtbrowserplugin.cpp
	interceptor.cpp
	)
SET (HEADERS
	qtbrowserplugin.h
	qtbrowserplugin_p.h
	qtnpapi.h
	interceptor.h
	)
IF (UNIX)
	LIST (APPEND SRCS qtbrowserplugin_x11.cpp)
ENDIF (UNIX)
IF (APPLE)
	LIST (APPEND SRCS qtbrowserplugin_mac.cpp)
ENDIF (APPLE)
IF (WIN32)
	LIST (APPEND SRCS qtbrowserplugin_win.cpp)
ENDIF (WIN32)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})

ADD_LIBRARY (lcnp SHARED
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES (lcnp
	${QT_LIBRARIES}
	)
