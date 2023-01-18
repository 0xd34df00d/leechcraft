cmake_minimum_required (VERSION 3.10)
project (aggregator)
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
		components/actions/appwideactions.cpp
		components/actions/channelactions.cpp
		components/actions/itemactions.cpp
		components/gui/itemnavigator.cpp
		components/gui/itemselectiontracker.cpp
		components/parsers/atom.cpp
		components/parsers/mediarss.cpp
		components/parsers/parse.cpp
		components/parsers/rss.cpp
		components/parsers/utils.cpp
		aggregator.cpp
		aggregatortab.cpp
		addfeeddialog.cpp
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
		feedsettings.cpp
		jobholderrepresentation.cpp
		itemswidget.cpp
		itemslistmodel.cpp
		startupfirstpage.cpp
		startupsecondpage.cpp
		startupthirdpage.cpp
		export2fb2dialog.cpp
		pluginmanager.cpp
		proxyobject.cpp
		dbupdatethread.cpp
		dumbstorage.cpp
		storagebackendmanager.cpp
		channelsmodelrepresentationproxy.cpp
		exportutils.cpp
		itemutils.cpp
		representationmanager.cpp
		updatesmanager.cpp
		resourcesfetcher.cpp
		opmladder.cpp
		poolsmanager.cpp
		feedserrormanager.cpp
		dbutils.cpp
		tooltipbuilder.cpp
	SETTINGS aggregatorsettings.xml
	RESOURCES aggregatorresources.qrc
	QT_COMPONENTS Network PrintSupport Sql Widgets Xml
	INSTALL_SHARE
	HAS_TESTS
	)

SUBPLUGIN (BODYFETCH "Enable BodyFetch for fetching full bodies of news items" OFF)
SUBPLUGIN (WEBACCESS "Enable WebAccess for providing HTTP access to Aggregator" OFF)

AddAggregatorTest (parsers_utils components/parsers/tests/utils_test)
