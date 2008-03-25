INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR}
    ${QT_QTXML_INCLUDE_DIR}
    ${Boost_INCLUDE_DIRS}
                                ${CMAKE_CURRENT_BINARY_DIR} ../.. .)

SET (SRCS
    aggregator.cpp
    core.cpp
    itemsmodel.cpp
    addfeed.cpp
    parserfactory.cpp
    rss20parser.cpp
    atom10parser.cpp
    parser.cpp
    item.cpp
    channel.cpp
    feed.cpp
    treeitem.cpp
    xmlsettingsmanager.cpp
    channelsmodel.cpp
    channelsfiltermodel.cpp)
SET (HEADERS
    aggregator.h
    core.h
    itemsmodel.h
    addfeed.h
    item.h
    channel.h
    feed.h
    enclosure.h
    parserfactory.h
    parser.h
    rss20parser.h
    atom10parser.h
    channelsmodel.h
    treeitem.h
    xmlsettingsmanager.h
    channelsfiltermodel.h
    ../../plugininterface/basesettingsmanager.h)
SET (FORMS
    mainwindow.ui
    addfeed.ui)
SET (RESOURCES resources.qrc)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})
ADD_LIBRARY (leechcraft_aggregator SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS})
LINK_DIRECTORIES (${Boost_LIBRARY_DIR})
TARGET_LINK_LIBRARIES (leechcraft_aggregator
                        ${QT_LIBRARIES}
                        boost_date_time
                        plugininterface
                        exceptions
                        xmlsettingsdialog)
INSTALL (TARGETS leechcraft_aggregator DESTINATION share/leechcraft/plugins)
