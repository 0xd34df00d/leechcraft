cmake_minimum_required (VERSION 3.10)
project (aggregator)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		components/actions/appwideactions.cpp
		components/actions/channelactions.cpp
		components/actions/itemactions.cpp
		components/gui/itemcategoryselector.cpp
		components/gui/itemnavigator.cpp
		components/gui/itemselectiontracker.cpp
		components/gui/util.cpp
		components/importexport/channelsexportmodel.cpp
		components/importexport/exportutils.cpp
		components/importexport/feedsexportdialog.cpp
		components/importexport/importopmldialog.cpp
		components/importexport/itemsexportdialog.cpp
		components/importexport/opmladder.cpp
		components/importexport/opmlparser.cpp
		components/importexport/writefb2.cpp
		components/importexport/writeopml.cpp
		components/importexport/writepdf.cpp
		components/itemrender/item.cpp
		components/itemrender/mrss.cpp
		components/itemrender/utils.cpp
		components/models/channelsfiltermodel.cpp
		components/models/channelsmodel.cpp
		components/models/itemscategoriestracker.cpp
		components/models/itemsfiltermodel.cpp
		components/models/itemslistmodel.cpp
		components/models/jobholderrepresentationmodel.cpp
		components/parsers/atom.cpp
		components/parsers/mediarss.cpp
		components/parsers/parse.cpp
		components/parsers/rss.cpp
		components/parsers/utils.cpp
		components/storage/dumbstorage.cpp
		components/storage/sqlstoragebackend.cpp
		components/storage/storagebackend.cpp
		components/storage/storagebackendmanager.cpp
		aggregator.cpp
		aggregatortab.cpp
		addfeeddialog.cpp
		item.cpp
		channel.cpp
		feed.cpp
		xmlsettingsmanager.cpp
		feedsettings.cpp
		itemswidget.cpp
		startupfirstpage.cpp
		startupsecondpage.cpp
		startupthirdpage.cpp
		pluginmanager.cpp
		proxyobject.cpp
		dbupdatethread.cpp
		representationmanager.cpp
		updatesmanager.cpp
		resourcesfetcher.cpp
		poolsmanager.cpp
		feedserrormanager.cpp
		dbutils.cpp
		tooltipbuilder.cpp
		common.cpp
	SETTINGS aggregatorsettings.xml
	RESOURCES aggregatorresources.qrc
	QT_COMPONENTS Network PrintSupport Sql Widgets Xml
	INSTALL_SHARE
	HAS_TESTS
	)

SUBPLUGIN (BODYFETCH "Enable BodyFetch for fetching full bodies of news items" OFF)
SUBPLUGIN (WEBACCESS "Enable WebAccess for providing HTTP access to Aggregator" OFF)

AddAggregatorTest (parsers_utils components/parsers/tests/utils_test)
