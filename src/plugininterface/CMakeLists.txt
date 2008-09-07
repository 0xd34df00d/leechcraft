INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
  proxy.cpp
  tagscompleter.cpp
  tagscompletionmodel.cpp
  treeitem.cpp
  tagslineedit.cpp
  graphwidget.cpp
  util.cpp
  )
SET (HEADERS
  proxy.h
  tagscompleter.h
  tagscompletionmodel.h
  treeitem.h
  tagslineedit.h
  graphwidget.h
  util.h
  )

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (plugininterface SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (plugininterface
                        ${QT_LIBRARIES}
                        exceptions)
INSTALL (TARGETS plugininterface DESTINATION lib)
