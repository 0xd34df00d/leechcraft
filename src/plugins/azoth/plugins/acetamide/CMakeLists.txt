cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_acetamide)
include (InitLCPlugin OPTIONAL)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (ACETAMIDE_SRCS
	acetamide.cpp
	bookmarkeditwidget.cpp
	channelclentry.cpp
	channelconfigwidget.cpp
	channelhandler.cpp
	channelparticipantentry.cpp
	channelpublicmessage.cpp
	channelslistdialog.cpp
	channelslistfilterproxymodel.cpp
	channelsmanager.cpp
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
	ircparticipantentry.cpp
	ircprotocol.cpp
	ircserversocket.cpp
	ircserverclentry.cpp
	ircserverhandler.cpp
	invitechannelsdialog.cpp
	localtypes.cpp
	newnickservidentifydialog.cpp
	nickservidentifywidget.cpp
	rplisupportparser.cpp
	servercommandmessage.cpp
	serverinfowidget.cpp
	serverparticipantentry.cpp
	serverresponsemanager.cpp
	sortfilterproxymodel.cpp
	usercommandmanager.cpp
	xmlsettingsmanager.cpp
	vcarddialog.cpp
	)

set (ACETAMIDE_FORMS
	bookmarkeditwidget.ui
	channelconfigwidget.ui
	channelslistdialog.ui
	ircaccountconfigurationdialog.ui
	ircaccountconfigurationwidget.ui
	ircjoingroupchat.ui
	invitechannelsdialog.ui
	newnickservidentifydialog.ui
	nickservidentifywidget.ui
	serverinfowidget.ui
	vcarddialog.ui
	)
set (ACETAMIDE_RESOURCES azothacetamideresources.qrc)

CreateTrs ("azoth_acetamide" "de;en;es;fr;it;ru_RU;uk_UA" ACETAMIDE_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_acetamide" "en;ru_RU" "${ACETAMIDE_SRCS}" "${ACETAMIDE_FORMS}" "azothacetamidesettings.xml")

QtWrapUi (ACETAMIDE_UIS_H ${ACETAMIDE_FORMS})
QtAddResources (ACETAMIDE_RCCS ${ACETAMIDE_RESOURCES})

add_library (leechcraft_azoth_acetamide SHARED
	${ACETAMIDE_COMPILED_TRANSLATIONS}
	${ACETAMIDE_SRCS}
	${ACETAMIDE_MOC_SRCS}
	${ACETAMIDE_UIS_H}
	${ACETAMIDE_RCCS}
	)
target_link_libraries (leechcraft_azoth_acetamide
	${LEECHCRAFT_LIBRARIES}
	)

FindQtLibs (leechcraft_azoth_acetamide Network Widgets Xml)

install (TARGETS leechcraft_azoth_acetamide DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothacetamidesettings.xml DESTINATION ${LC_SETTINGS_DEST})
if (UNIX AND NOT APPLE)
	if (USE_QT5)
		install (FILES freedesktop/leechcraft-azoth-acetamide-qt5.desktop DESTINATION share/applications)
	else ()
		install (FILES freedesktop/leechcraft-azoth-acetamide.desktop DESTINATION share/applications)
	endif ()
endif ()
