set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (MAGIC)
if (MAGIC_FOUND)
	add_definitions (-DHAVE_MAGIC)
else ()
	set (MAGIC_INCLUDE_DIRS)
	set (MAGIC_LIBRARIES)
endif ()

option (WITH_X11 "Enable X11 support on Linux" ON)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${MAGIC_INCLUDE_DIRS}
	..)

set (SRCS
	gui/autoresizemixin.cpp
	gui/clearlineeditaddon.cpp
	gui/findnotification.cpp
	gui/flowlayout.cpp
	gui/pagenotification.cpp
	gui/selectablebrowser.cpp
	gui/unhoverdeletemixin.cpp
	gui/util.cpp
	models/flattenfiltermodel.cpp
	models/flattofoldersproxymodel.cpp
	models/listmodel.cpp
	models/mergemodel.cpp
	models/modeliterator.cpp
	models/treeitem.cpp
	network/customcookiejar.cpp
	network/customnetworkreply.cpp
	network/networkdiskcache.cpp
	network/socketerrorstrings.cpp
	qml/colorthemeproxy.cpp
	qml/settableiconprovider.cpp
	qml/standardnamfactory.cpp
	qml/themeimageprovider.cpp
	qml/tooltipitem.cpp
	qml/unhidelistmodel.cpp
	qml/unhidelistviewbase.cpp
	qml/widthiconprovider.cpp
	svcauth/vkauthmanager.cpp
	sys/mimedetector.cpp
	sys/paths.cpp
	xdg/desktopparser.cpp
	xdg/item.cpp
	xdg/itemsdatabase.cpp
	xdg/itemsfinder.cpp
	xdg/itemtypes.cpp
	xdg/xdg.cpp
	tags/categoryselector.cpp
	tags/tagscompleter.cpp
	tags/tagscompletionmodel.cpp
	tags/tagslineedit.cpp
	tags/tagsfiltermodel.cpp
	xpc/stdanfields.cpp
	xpc/stddatafiltermenucreator.cpp
	xpc/util.cpp
	util.cpp
	dblock.cpp
	backendselector.cpp
	structuresops.cpp
	defaulthookproxy.cpp
	basehookinterconnector.cpp
	fileremoveguard.cpp
	exceptions.cpp
	resourceloader.cpp
	notificationactionhandler.cpp
	sysinfo.cpp
	passutils.cpp
	queuemanager.cpp
	delayedexecutor.cpp
	shortcuts/shortcutmanager.cpp
	)
set (FORMS
	backendselector.ui
	gui/findnotification.ui
	)

QtWrapUi (UIS_H ${FORMS})

if (WITH_X11 AND NOT APPLE)
	find_package (X11)

	if (X11_FOUND)
		add_definitions (-DHAVE_X11)
		set (ADDITIONAL_LIBS ${X11_X11_LIB})

		set (SRCS ${SRCS} x11/xwrapper.cpp)
	endif ()
endif ()

foreach (SRC ${SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/${SRC}")
endforeach ()
foreach (FORM ${FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/${FORM}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_definitions(${QT_DEFINITIONS})
add_definitions(-DQT_PLUGIN)
add_definitions(-DQT_NO_DEBUG)
add_definitions(-DQT_SHARED)
add_definitions(-DQT_GUI_LIB)

add_library (leechcraft-util SHARED
	${SRCS}
	${UIS_H}
	)

FindQtLibs (leechcraft-util Concurrent Quick Qml Network Xml Sql WebKitWidgets Widgets X11Extras)
target_link_libraries (leechcraft-util
	${QT_LIBRARIES}
	${MAGIC_LIBRARIES}
	${ADDITIONAL_LIBS}
	leechcraft-xsd
	)

set_property (TARGET leechcraft-util PROPERTY SOVERSION 0.5.0)
install (TARGETS leechcraft-util DESTINATION ${LIBDIR})
