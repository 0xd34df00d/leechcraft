INCLUDE (${CMAKE_ROOT}/Modules/FindQt4.cmake)
INCLUDE (${CMAKE_ROOT}/Modules/UseQt4.cmake)
INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ../..)

SET (SRCS
	 settingsdialog.cpp
	 settingsiteminfo.cpp
	 typehandler.cpp
	 typelist.cpp
	 converters.cpp
	 filepickerwidget.cpp
	 datatypes.cpp
	 extendedspinbox.cpp
	 collectorwidget.cpp)
SET (HEADERS
	 settingsdialog.h
	 settingsiteminfo.h
	 typehandler.h
	 typelist.h
	 converters.h
	 filepickerwidget.h
	 datatypes.h
	 extendedspinbox.h
	 collectorwidget.h)

ADD_DEFINITIONS(${QT_DEFINITIONS})
ADD_DEFINITIONS(-DQT_PLUGIN)
ADD_DEFINITIONS(-DQT_NO_DEBUG)
ADD_DEFINITIONS(-DQT_SHARED)
ADD_DEFINITIONS(-DQT_GUI_LIB)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (settingsdialog SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (settingsdialog
                        ${QT_LIBRARIES}
                        exceptions)
INSTALL (TARGETS settingsdialog DESTINATION lib)
