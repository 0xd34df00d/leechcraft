cmake_minimum_required (VERSION 2.8)
project (leechcraft_aggregator)
include (InitLCPlugin OPTIONAL)

option (ENABLE_AGGREGATOR_BODYFETCH "Enable BodyFetch for fetching full bodies of news items" ON)
option (ENABLE_AGGREGATOR_WEBACCESS "Enable WebAccess for providing HTTP access to Aggregator" OFF)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
    aggregator.cpp
    core.cpp
    addfeed.cpp
    parserfactory.cpp
	rssparser.cpp
	rss20parser.cpp
	rss10parser.cpp
	rss091parser.cpp
	atomparser.cpp
	atom10parser.cpp
	atom03parser.cpp
	parser.cpp
	item.cpp
	channel.cpp
	feed.cpp
	xmlsettingsmanager.cpp
	uistatepersist.cpp
	channelsmodel.cpp
	channelsfiltermodel.cpp
	regexpmatcherui.cpp
	singleregexp.cpp
	regexpmatchermanager.cpp
	itemsfiltermodel.cpp
	opmlparser.cpp
	opmlwriter.cpp
	importopml.cpp
	export.cpp
	sqlstoragebackend.cpp
	storagebackend.cpp
	importbinary.cpp
	feedsettings.cpp
	jobholderrepresentation.cpp
	itemswidget.cpp
	itemslistmodel.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	startupsecondpage.cpp
	startupthirdpage.cpp
	export2fb2dialog.cpp
	actionsstructs.cpp
	sqlstoragebackend_mysql.cpp
	pluginmanager.cpp
	proxyobject.cpp
	dbupdatethread.cpp
	dbupdatethreadworker.cpp
	tovarmaps.cpp
	)
set (HEADERS
    aggregator.h
    core.h
    addfeed.h
    item.h
    channel.h
    feed.h
    parserfactory.h
    parser.h
	rssparser.h
	rss20parser.h
	rss10parser.h
	rss091parser.h
	atomparser.h
	atom10parser.h
	atom03parser.h
	channelsmodel.h
	xmlsettingsmanager.h
	uistatepersist.h
	channelsfiltermodel.h
	regexpmatcherui.h
	singleregexp.h
	regexpmatchermanager.h
	itemsfiltermodel.h
	opmlparser.h
	opmlwriter.h
	importopml.h
	export.h
	sqlstoragebackend.h
	storagebackend.h
	opmlitem.h
	importbinary.h
	feedsettings.h
	jobholderrepresentation.h
	itemswidget.h
	itemslistmodel.h
	wizardgenerator.h
	startupfirstpage.h
	startupsecondpage.h
	startupthirdpage.h
	export2fb2dialog.h
	actionsstructs.h
	sqlstoragebackend_mysql.h
	pluginmanager.h
	proxyobject.h
	dbupdatethread.h
	dbupdatethreadworker.h
	tovarmaps.h
	)
set (FORMS
    mainwidget.ui
    addfeed.ui
	itembucket.ui
	regexpmatcherui.ui
	singleregexp.ui
	importopml.ui
	export.ui
	importbinary.ui
	feedsettings.ui
	itemswidget.ui
	startupfirstpage.ui
	startupsecondpage.ui
	startupthirdpage.ui
	export2fb2dialog.ui
	)
set (RESOURCES aggregatorresources.qrc)
CreateTrs ("aggregator" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("aggregator" "en;ru_RU" "${SRCS}" "${FORMS}" "aggregatorsettings.xml")

add_definitions(-DBOOST_DYN_LINK)
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_aggregator SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

FindQtLibs (leechcraft_aggregator Gui Widgets Sql Xml Network WebKitWidgets)

link_directories (${Boost_LIBRARY_DIR})
target_link_libraries (leechcraft_aggregator
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_aggregator DESTINATION ${LC_PLUGINS_DEST})
install (FILES aggregatorsettings.xml DESTINATION ${LC_SETTINGS_DEST})

set (AGGREGATOR_INCLUDE_DIR ${CURRENT_SOURCE_DIR})

if (ENABLE_AGGREGATOR_BODYFETCH)
	add_subdirectory (plugins/bodyfetch)
endif ()

if (ENABLE_AGGREGATOR_WEBACCESS)
	add_subdirectory (plugins/webaccess)
endif ()
