SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTSQL TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR})

SET (SRCS
	proxy.cpp
	tagscompleter.cpp
	tagscompletionmodel.cpp
	treeitem.cpp
	tagslineedit.cpp
	tagsfiltermodel.cpp
	graphwidget.cpp
	util.cpp
	categoryselector.cpp
	historymodel.cpp
	mergemodel.cpp
	dblock.cpp
	customcookiejar.cpp
	)
SET (HEADERS
	proxy.h
	tagscompleter.h
	tagscompletionmodel.h
	treeitem.h
	tagslineedit.h
	tagsfiltermodel.h
	graphwidget.h
	util.h
	categoryselector.h
	historymodel.h
	mergemodel.h
	dblock.h
	customcookiejar.h
	)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (plugininterface SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (plugininterface
	${QT_LIBRARIES}
	)
INSTALL (TARGETS plugininterface DESTINATION lib)
