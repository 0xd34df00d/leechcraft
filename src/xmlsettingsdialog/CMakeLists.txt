INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
                     ${QT_QTCORE_INCLUDE_DIR}
                     ${QT_QTGUI_INCLUDE_DIR}
                     ${QT_QTXML_INCLUDE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
    xmlsettingsdialog.cpp
 rangewidget.cpp
 filepicker.cpp
 radiogroup.cpp)
SET (HEADERS
    xmlsettingsdialog.h
 rangewidget.h
 filepicker.h
 radiogroup.h)
ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (xmlsettingsdialog SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (xmlsettingsdialog
                        ${QT_LIBRARIES}
                        plugininterface
                        exceptions)
INSTALL (TARGETS xmlsettingsdialog DESTINATION lib)
