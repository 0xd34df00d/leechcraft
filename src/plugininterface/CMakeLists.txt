SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	..)

SET (SRCS
	tagscompleter.cpp
	tagscompletionmodel.cpp
	treeitem.cpp
	tagslineedit.cpp
	tagsfiltermodel.cpp
	util.cpp
	categoryselector.cpp
	mergemodel.cpp
	dblock.cpp
	customcookiejar.cpp
	modeltest.cpp
	backendselector.cpp
	structuresops.cpp
	listmodel.cpp
	selectablebrowser.cpp
	flattofoldersproxymodel.cpp
	)
SET (HEADERS
	tagscompleter.h
	tagscompletionmodel.h
	treeitem.h
	tagslineedit.h
	tagsfiltermodel.h
	util.h
	categoryselector.h
	mergemodel.h
	dblock.h
	customcookiejar.h
	modeltest.h
	backendselector.h
	structuresops.h
	listmodel.h
	selectablebrowser.h
	flattofoldersproxymodel.h
	)
SET (FORMS
	backendselector.ui
	)
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (plugininterface SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (plugininterface
	${QT_LIBRARIES}
	xmlsettingsdialog
	)
SET_PROPERTY (TARGET plugininterface PROPERTY SOVERSION 0.3.0)
INSTALL (TARGETS plugininterface DESTINATION ${LIBDIR})
