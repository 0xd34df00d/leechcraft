cmake_minimum_required (VERSION 3.10)
project (newlife)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

LC_DEFINE_PLUGIN (
	SRCS
		newlife.cpp
		importwizard.cpp
		firstpage.cpp
		abstractimporter.cpp
		imhistimporterbase.cpp
		entitygeneratingpage.cpp
		common/imimporter.cpp
		common/imimportpage.cpp
		common/xmlimaccount.cpp
		importers/akregator/akregatorimporter.cpp
		importers/akregator/akregatorimportpage.cpp
		importers/liferea/lifereaimportpage.cpp
		importers/liferea/lifereaimporter.cpp
		importers/kopete/kopeteimporter.cpp
		importers/kopete/kopeteimportpage.cpp
		importers/kopete/kopeteimportthread.cpp
		importers/ktorrent/ktorrentimportpage.cpp
		importers/ktorrent/ktorrentimporter.cpp
		importers/firefox/firefoximportpage.cpp
		importers/firefox/firefoximporter.cpp
		importers/firefox/firefoxprofileselectpage.cpp
		importers/psiplus/psiplusimporter.cpp
		importers/psiplus/psiplusimportpage.cpp
		importers/vacuum/vacuumimporter.cpp
		importers/vacuum/vacuumimportpage.cpp
		importers/opera/operaimporter.cpp
		importers/opera/operaimportselectpage.cpp
		importers/jsonbookmarks/jsonbookmarksimporter.cpp
		importers/jsonbookmarks/jsonbookmarksimportpage.cpp
	QT_COMPONENTS Sql Widgets Xml
	INSTALL_SHARE
	)
