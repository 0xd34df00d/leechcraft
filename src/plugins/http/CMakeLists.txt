INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR}
 ${CMAKE_CURRENT_BINARY_DIR} ../.. .)

SET (SRCS
 httpplugin.cpp 
 jobadderdialog.cpp 
 jobparams.cpp 
 job.cpp 
 httpimp.cpp 
 ftpimp.cpp 
 jobmanager.cpp 
 joblistitem.cpp 
 fileexistsdialog.cpp 
 finishedjob.cpp 
 impbase.cpp 
 mainviewdelegate.cpp 
 contextablelist.cpp
 columnselector.cpp
 jobpropertiesdialog.cpp
 xmlsettingsmanager.cpp)
SET (HEADERS
 httpplugin.h
 jobadderdialog.h
 jobparams.h
 job.h
 httpimp.h
 ftpimp.h
 jobmanager.h
 joblistitem.h
 fileexistsdialog.h
 finishedjob.h
 xmlsettingsmanager.h
 impbase.h
 mainviewdelegate.h
 contextablelist.h
 columnselector.h
 jobpropertiesdialog.h
 globals.h
 ../../plugininterface/basesettingsmanager.h)
SET (FORMS
 fileexistsdialog.ui
 columnselector.ui
 jobadderdialog.ui
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
ADD_LIBRARY (leechcraft_http SHARED ${SRCS} ${MOC_SRCS} ${UIS_H} ${RCCS})
TARGET_LINK_LIBRARIES (leechcraft_http
                        ${QT_LIBRARIES}
                        plugininterface
                        exceptions
                        xmlsettingsdialog)
INSTALL (TARGETS leechcraft_http DESTINATION share/leechcraft/plugins)
