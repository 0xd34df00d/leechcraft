CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
IF (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (leechcraft_azoth_server)
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/GX /W3")
	SET (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD")
	SET (CMAKE_EXE_LINKER_FLAGS "/entry:mainCRTStartup")
ENDIF (MSVC)
IF (UNIX)
	SET (CMAKE_CXX_FLAGS "-fvisibility=hidden")
	IF (EXTRA_WARNINGS)
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	ENDIF (EXTRA_WARNINGS)
	IF (USE_PIPE)
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
	ENDIF (USE_PIPE)
ENDIF (UNIX)
IF (WIN32 AND NOT MSVC)
	SET (CMAKE_CXX_FLAGS "-Wall")
ENDIF (WIN32 AND NOT MSVC)

SET (QT_USE_QTDBUS TRUE)
SET (QT_USE_QTGUI FALSE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	)

SET (SRCS
	main.cpp
	core.cpp
	azothclientconnection.cpp
	azothclientconnectionadaptor.cpp
	)
SET (HEADERS
	core.h
	azothclientconnection.h
	azothclientconnectionadaptor.h
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})

ADD_EXECUTABLE (leechcraft_azoth_server WIN32
	${SRCS}
	${MOC_SRCS}
	)
TARGET_LINK_LIBRARIES(leechcraft_azoth_server
						${QT_LIBRARIES})
INSTALL (TARGETS leechcraft_azoth_server DESTINATION bin)
