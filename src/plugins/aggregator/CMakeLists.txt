INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR}
                                ${CMAKE_CURRENT_BINARY_DIR} ../.. .)

SET (SRCS
    aggregator.cpp
    core.cpp)
SET (HEADERS
    aggregator.h
    core.h
    ../../plugininterface/basesettingsmanager.h)
SET (FORMS
    mainwindow.ui)
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
TARGET_LINK_LIBRARIES (leechcraft_aggregator
                        ${QT_LIBRARIES}
                        plugininterface
                        exceptions
                        xmlsettingsdialog)
INSTALL (TARGETS leechcraft_aggregator DESTINATION share/leechcraft/plugins)
