INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
	${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
	${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${QT_QTWEBKIT_INCLUDE_DIR}
	${QT_QTSQL_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR} ../.. .)

SET (SRCS
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
    channelsmodel.cpp
    channelsfiltermodel.cpp
	browserwidget.cpp
	itembucket.cpp
	itemmodel.cpp
	regexpmatcherui.cpp
	singleregexp.cpp
	regexpmatchermanager.cpp
	itemsfiltermodel.cpp
	opmlparser.cpp
	opmlwriter.cpp
	importopml.cpp
	exportopml.cpp
	sqlstoragebackend.cpp
	storagebackend.cpp
	)
SET (HEADERS
    aggregator.h
    core.h
    addfeed.h
    item.h
    channel.h
    feed.h
    enclosure.h
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
    channelsfiltermodel.h
	browserwidget.h
	itembucket.h
	itemmodel.h
	regexpmatcherui.h
	singleregexp.h
	regexpmatchermanager.h
	itemsfiltermodel.h
	opmlparser.h
	opmlwriter.h
	importopml.h
	exportopml.h
	sqlstoragebackend.h
	storagebackend.h
	opmlitem.h
	../../xmlsettingsdialog/basesettingsmanager.h
	)
SET (FORMS
    mainwidget.ui
    addfeed.ui
	browserwidget.ui
	itembucket.ui
	regexpmatcherui.ui
	singleregexp.ui
	importopml.ui
	exportopml.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_aggregator_ru_RU.qm
	leechcraft_aggregator_uk_UA.qm
	)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_TARGET (aggregator_translations
	COMMAND "lrelease" "leechcraft_aggregator_ru_RU.ts" "leechcraft_aggregator_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_LIBRARY (leechcraft_aggregator SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS})
ADD_DEPENDENCIES (leechcraft_aggregator aggregator_translations)
LINK_DIRECTORIES (${Boost_LIBRARY_DIR})
TARGET_LINK_LIBRARIES (leechcraft_aggregator
                        ${QT_LIBRARIES}
                        plugininterface
                        xmlsettingsdialog)
INSTALL (TARGETS leechcraft_aggregator DESTINATION ${LC_PLUGINS_DEST})

IF (UNIX)
	INSTALL (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor
		PATTERN ".svn" EXCLUDE)
ENDIF (UNIX)
