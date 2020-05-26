cmake_minimum_required (VERSION 3.3)
project (leechcraft_aggregator)
include (InitLCPlugin NO_POLICY_SCOPE)

set (UTIL_SRCS
	util/aggregator/itemsmodeldecorator.cpp
	)
add_library (leechcraft_aggregator_util STATIC
	${UTIL_SRCS}
	)
set_target_properties(leechcraft_aggregator_util PROPERTIES POSITION_INDEPENDENT_CODE True)
FindQtLibs (leechcraft_aggregator_util Widgets)

LC_DEFINE_PLUGIN (
	SRCS
		aggregator.cpp
		aggregatortab.cpp
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
		poolsmanager.cpp
		coreproxyholder.cpp
		feedserrormanager.cpp
	SETTINGS aggregatorsettings.xml
	RESOURCES aggregatorresources.qrc
	QT_COMPONENTS Network PrintSupport Sql Widgets Xml
	INSTALL_SHARE
	)

SUBPLUGIN (BODYFETCH "Enable BodyFetch for fetching full bodies of news items")
SUBPLUGIN (WEBACCESS "Enable WebAccess for providing HTTP access to Aggregator" OFF)
