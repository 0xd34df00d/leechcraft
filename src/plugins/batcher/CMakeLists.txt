SET (QT_USE_QTNETWORK true)
INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
    batcher.cpp
	parser.cpp)
SET (HEADERS
    batcher.h
	parser.h)
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
ADD_LIBRARY (leechcraft_batcher SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS})
TARGET_LINK_LIBRARIES (leechcraft_batcher
                        ${QT_LIBRARIES}
                        plugininterface
                        exceptions
                        settingsdialog)
INSTALL (TARGETS leechcraft_batcher DESTINATION share/leechcraft/plugins)
