SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

FIND_PACKAGE (Gloox REQUIRED)

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${GLOOX_INCLUDE_DIR})
SET (XOOX_SRCS
	xoox.cpp
	glooxprotocol.cpp
	glooxaccountconfigurationdialog.cpp
	glooxaccount.cpp
	clientconnection.cpp
	glooxclentry.cpp
	glooxmessage.cpp
	)
SET (XOOX_HEADERS
	xoox.h
	glooxprotocol.h
	glooxaccountconfigurationdialog.h
	glooxaccount.h
	clientconnection.h
	glooxclentry.h
	glooxmessage.h
	)
SET (XOOX_FORMS
	glooxaccountconfigurationdialog.ui
	)
QT4_WRAP_CPP (XOOX_MOC_SRCS ${XOOX_HEADERS})
QT4_WRAP_UI (XOOX_UIS_H ${XOOX_FORMS})

ADD_LIBRARY (leechcraft_azoth_xoox SHARED
	${XOOX_SRCS}
	${XOOX_MOC_SRCS}
	${XOOX_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_xoox
	${QT_LIBRARIES}
	${GLOOX_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_azoth_xoox DESTINATION ${LC_PLUGINS_DEST})
