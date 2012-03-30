CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_xproxy)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	xproxy.cpp
	proxyfactory.cpp
	xmlsettingsmanager.cpp
	proxiesconfigwidget.cpp
	)
SET (HEADERS
	xproxy.h
	proxyfactory.h
	xmlsettingsmanager.h
	proxiesconfigwidget.h
	)
SET (FORMS
	proxiesconfigwidget.ui
	)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})

ADD_LIBRARY (leechcraft_xproxy SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_xproxy
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_xproxy DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES xproxysettings.xml DESTINATION ${LC_SETTINGS_DEST})
