SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	..)

SET (SRCS
	gui/flowlayout.cpp
	gui/selectablebrowser.cpp
	models/flattofoldersproxymodel.cpp
	models/listmodel.cpp
	models/mergemodel.cpp
	models/treeitem.cpp
	sync/syncdeltamanager.cpp
	sync/syncops.cpp
	tags/categoryselector.cpp
	tags/tagscompleter.cpp
	tags/tagscompletionmodel.cpp
	tags/tagslineedit.cpp
	tags/tagsfiltermodel.cpp
	util.cpp
	dblock.cpp
	customcookiejar.cpp
	backendselector.cpp
	structuresops.cpp
	defaulthookproxy.cpp
	basehookinterconnector.cpp
	fileremoveguard.cpp
	exceptions.cpp
	customnetworkreply.cpp
	resourceloader.cpp
	notificationactionhandler.cpp
	socketerrorstrings.cpp
	sysinfo.cpp
	passutils.cpp
	shortcuts/shortcutmanager.cpp
	)
SET (HEADERS
	gui/flowlayout.h
	gui/selectablebrowser.h
	models/flattofoldersproxymodel.h
	models/listmodel.h
	models/mergemodel.h
	models/treeitem.h
	sync/syncdeltamanager.h
	sync/syncops.h
	tags/categoryselector.h
	tags/tagscompleter.h
	tags/tagscompletionmodel.h
	tags/tagslineedit.h
	tags/tagsfiltermodel.h
	util.h
	dblock.h
	customcookiejar.h
	backendselector.h
	structuresops.h
	defaulthookproxy.h
	basehookinterconnector.h
	fileremoveguard.h
	exceptions.h
	customnetworkreply.h
	resourceloader.h
	notificationactionhandler.h
	socketerrorstrings.h
	sysinfo.h
	passutils.h
	shortcuts/shortcutmanager.h
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
