cmake_minimum_required (VERSION 3.3)
project (leechcraft_aggregator)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_AGGREGATOR_BODYFETCH "Enable BodyFetch for fetching full bodies of news items" ON)
option (ENABLE_AGGREGATOR_WEBACCESS "Enable WebAccess for providing HTTP access to Aggregator" OFF)

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	aggregator.cpp
	aggregatortab.cpp
	core.cpp
	addfeeddialog.cpp
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
	startupfirstpage.cpp
	startupsecondpage.cpp
	startupthirdpage.cpp
	export2fb2dialog.cpp
	actionsstructs.cpp
	pluginmanager.cpp
	proxyobject.cpp
	dbupdatethread.cpp
	dbupdatethreadworker.cpp
	dumbstorage.cpp
	storagebackendmanager.cpp
	channelsmodelrepresentationproxy.cpp
	exportutils.cpp
	itemutils.cpp
	channelutils.cpp
	representationmanager.cpp
	updatesmanager.cpp
	resourcesfetcher.cpp
	opmladder.cpp
	)
set (FORMS
	mainwidget.ui
	addfeeddialog.ui
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

set (UTIL_SRCS
	util/aggregator/itemsmodeldecorator.cpp
	)

CreateTrs ("aggregator" "ar;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("aggregator" "en;ru_RU" "${SRCS}" "${FORMS}" "aggregatorsettings.xml")

add_library (leechcraft_aggregator_util STATIC
	${UTIL_SRCS}
	)
set_target_properties(leechcraft_aggregator_util PROPERTIES POSITION_INDEPENDENT_CODE True)

FindQtLibs (leechcraft_aggregator_util Core Widgets)

add_definitions(-DBOOST_DYN_LINK)
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_aggregator SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
link_directories (${Boost_LIBRARY_DIR})
target_link_libraries (leechcraft_aggregator
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_aggregator DESTINATION ${LC_PLUGINS_DEST})
install (FILES aggregatorsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_aggregator Network PrintSupport Sql Widgets Xml)

set (AGGREGATOR_INCLUDE_DIR ${CURRENT_SOURCE_DIR})

if (ENABLE_AGGREGATOR_BODYFETCH)
	add_subdirectory (plugins/bodyfetch)
endif ()

if (ENABLE_AGGREGATOR_WEBACCESS)
	add_subdirectory (plugins/webaccess)
endif ()
