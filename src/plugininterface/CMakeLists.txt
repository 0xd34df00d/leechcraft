SET (QT_USE_QTNETWORK true)
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
	 addressparser.cpp
	 proxy.cpp
	 tcpsocket.cpp
	 filewriter.cpp
	 filewriterthread.cpp
	 socketexceptions.cpp)
SET (HEADERS
	 addressparser.h
	 proxy.h
	 tcpsocket.h
	 filewriter.h
	 filewriterthread.h
	 socketexceptions.h)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (plugininterface SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (plugininterface
                        ${QT_LIBRARIES}
                        exceptions)
INSTALL (TARGETS plugininterface DESTINATION lib)
