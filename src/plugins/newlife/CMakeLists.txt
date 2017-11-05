cmake_minimum_required (VERSION 3.3)
project (leechcraft_newlife)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
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
	)
set (FORMS
	importwizard.ui
	feedssettingsimportpage.ui
	firstpage.ui
	common/imimportpage.ui
	importers/firefox/firefoxprofileselectpage.ui
	importers/opera/operaimportselectpage.ui
	importers/jsonbookmarks/jsonbookmarksimportpage.ui
	)
set (RESOURCES newliferesources.qrc)
CreateTrs ("newlife" "ar;be;de;en;fr;it;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("newlife" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_newlife SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_newlife
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_newlife DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_newlife Sql Widgets Xml)
