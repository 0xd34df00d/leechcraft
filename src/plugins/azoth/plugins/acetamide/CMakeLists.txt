SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

SET (ACETAMIDE_SRCS    
	acetamide.cpp
	core.cpp
	ircprotocol.cpp
	ircaccount.cpp
	ircaccountconfigurationdialog.cpp
	ircaddserverdialog.cpp
	ircadddefaultchannelsdialog.cpp
	irceditchanneldialog.cpp
	)

SET (ACETAMIDE_HEADERS
	acetamide.h
	core.h
	ircprotocol.h
	ircaccount.h
	ircaccountconfigurationdialog.h
	ircaddserverdialog.h
	ircadddefaultchannelsdialog.h
	irceditchanneldialog.h
	)

SET (ACETAMIDE_FORMS
	ircjoingroupchatdialog.ui
	ircaccountconfigurationdialog.ui
	ircadddefaultchannelsdialog.ui
	ircaddserverdialog.ui
	irceditchanneldialog.ui
	)

QT4_WRAP_CPP (ACETAMIDE_MOC_SRCS ${ACETAMIDE_HEADERS})
QT4_WRAP_UI (ACETAMIDE_UIS_H ${ACETAMIDE_FORMS})

ADD_LIBRARY (leechcraft_azoth_acetamide SHARED
	${ACETAMIDE_SRCS}
	${ACETAMIDE_MOC_SRCS}
	${ACETAMIDE_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_acetamide
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_azoth_acetamide DESTINATION ${LC_PLUGINS_DEST})
