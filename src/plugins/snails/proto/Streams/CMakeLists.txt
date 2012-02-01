SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/../
	)

SET (SRCS
	DeletionWatcher.cpp
	FakeSocket.cpp
	IODeviceSocket.cpp
	Socket.cpp
	SocketFactory.cpp
	)
SET (HEADERS
	DeletionWatcher.h
	FakeSocket.h
	IODeviceSocket.h
	Socket.h
	SocketFactory.h
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)

ADD_LIBRARY (Streams STATIC
	${SRCS}
	${MOC_SRCS}
	)