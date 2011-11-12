SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
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
	backendselector.cpp
	structuresops.cpp
	listmodel.cpp
	selectablebrowser.cpp
	flattofoldersproxymodel.cpp
	defaulthookproxy.cpp
	basehookinterconnector.cpp
	fileremoveguard.cpp
	exceptions.cpp
	syncops.cpp
	customnetworkreply.cpp
	resourceloader.cpp
	notificationactionhandler.cpp
	flowlayout.cpp
	socketerrorstrings.cpp
	syncdeltamanager.cpp
	sysinfo.cpp
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
	backendselector.h
	structuresops.h
	listmodel.h
	selectablebrowser.h
	flattofoldersproxymodel.h
	defaulthookproxy.h
	basehookinterconnector.h
	fileremoveguard.h
	exceptions.h
	syncops.h
	customnetworkreply.h
	resourceloader.h
	notificationactionhandler.h
	flowlayout.h
	socketerrorstrings.h
	syncdeltamanager.h
	sysinfo.h
	idpool.h
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
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
ADD_LIBRARY (lcutil SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (lcutil
	${QT_LIBRARIES}
	xmlsettingsdialog
	)
SET_PROPERTY (TARGET lcutil PROPERTY SOVERSION 0.5.0)
INSTALL (TARGETS lcutil DESTINATION ${LIBDIR})
