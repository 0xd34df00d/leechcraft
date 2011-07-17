SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	)

SET (ACETAMIDE_SRCS
	acetamide.cpp
	channelclentry.cpp
	channelhandler.cpp
	channelpublicmessage.cpp
	clientconnection.cpp
	core.cpp
	entrybase.cpp
	ircaccount.cpp
	ircaccountconfigurationdialog.cpp
	ircaccountconfigurationwidget.cpp
	ircerrorhandler.cpp
	ircjoingroupchat.cpp
	ircmessage.cpp
	ircparser.cpp
	ircprotocol.cpp
	ircserverclentry.cpp
	ircserverhandler.cpp
	invitechannelsdialog.cpp
	localtypes.cpp
	servercommandmessage.cpp
	serverparticipantentry.cpp
	xmlsettingsmanager.cpp
	)

SET (ACETAMIDE_HEADERS
	acetamide.h
	channelclentry.h
	channelhandler.h
	channelpublicmessage.h
	clientconnection.h
	core.h
	entrybase.h
	ircaccount.h
	ircaccountconfigurationdialog.h
	ircaccountconfigurationwidget.h
	ircerrorhandler.h
	ircjoingroupchat.h
	ircmessage.h
	ircparser.h
	ircprotocol.h
	ircserverclentry.h
	ircserverhandler.h
	invitechannelsdialog.h
	localtypes.h
	servercommandmessage.h
	serverparticipantentry.h
	xmlsettingsmanager.h
	)

SET (ACETAMIDE_FORMS
	ircaccountconfigurationdialog.ui
	ircaccountconfigurationwidget.ui
	ircjoingroupchat.ui
	invitechannelsdialog.ui
	)
SET (ACETAMIDE_RESOURCES azothacetamideresources.qrc)

CreateTrVars ("azoth_acetamide" "de;en;fr;it;ru_RU;uk_UA" ACETAMIDE_TRANSLATIONS ACETAMIDE_COMPILED_TRANSLATIONS)

QT4_WRAP_CPP (ACETAMIDE_MOC_SRCS ${ACETAMIDE_HEADERS})
QT4_WRAP_UI (ACETAMIDE_UIS_H ${ACETAMIDE_FORMS})
QT4_ADD_RESOURCES (ACETAMIDE_RCCS ${ACETAMIDE_RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${ACETAMIDE_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${ACETAMIDE_TRANSLATIONS}
	DEPENDS ${ACETAMIDE_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_acetamide SHARED
	${ACETAMIDE_COMPILED_TRANSLATIONS}
	${ACETAMIDE_SRCS}
	${ACETAMIDE_MOC_SRCS}
	${ACETAMIDE_UIS_H}
	${ACETAMIDE_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_acetamide
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_azoth_acetamide
		DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${ACETAMIDE_COMPILED_TRANSLATIONS}
		DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES azothacetamidesettings.xml
		DESTINATION ${LC_SETTINGS_DEST})
