IF (NOT QT_USE_FILE)
	CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
	IF (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	ENDIF (COMMAND cmake_policy)

	PROJECT (leechcraft_azoth)

	IF (NOT CMAKE_MODULE_PATH)
		SET (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	ENDIF (NOT CMAKE_MODULE_PATH)

	FIND_PACKAGE (Boost REQUIRED)
	FIND_PACKAGE (Qt4 REQUIRED)
	FIND_PACKAGE (LeechCraft REQUIRED)
ENDIF (NOT QT_USE_FILE)

SET (QT_USE_QTDBUS TRUE)
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTWEBKIT TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)

INCLUDE (${QT_USE_FILE})

FIND_PACKAGE (QtGStreamer REQUIRED)

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QTGSTREAMER_INCLUDE_DIR}
	${QTGSTREAMER_INCLUDE_DIR}/QGst
	${QTGSTREAMER_INCLUDES}
	)

OPTION (ENABLE_CRYPT "Enable QCA2-based support for PGP" ON)

IF (ENABLE_CRYPT)
	SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
	FIND_PACKAGE (QCA2)

	IF (NOT QCA2_FOUND)
		SET (ENABLE_CRYPT OFF)
		MESSAGE (WARNING "QCA2 is NOT found, support for PGP-signed/encrypted messages won't be built")

		SET (QCA2_INCLUDE_DIR "")
		SET (QCA2_LIBRARIES "")
	ENDIF (NOT QCA2_FOUND)

	INCLUDE_DIRECTORIES (${QCA2_INCLUDE_DIR})
ENDIF (ENABLE_CRYPT)

IF (ENABLE_CRYPT)
	ADD_DEFINITIONS (-DENABLE_CRYPT)
ENDIF (ENABLE_CRYPT)

SET (SRCS
	azoth.cpp
	core.cpp
	mainwidget.cpp
	chattabsmanager.cpp
	chattab.cpp
	sortfilterproxymodel.cpp
	accountslistwidget.cpp
	setstatusdialog.cpp
	pluginmanager.cpp
	proxyobject.cpp
	textedit.cpp
	xmlsettingsmanager.cpp
	contactlistdelegate.cpp
	addcontactdialog.cpp
	joinconferencedialog.cpp
	groupeditordialog.cpp
	transferjobmanager.cpp
	addaccountwizardfirstpage.cpp
	bookmarksmanagerdialog.cpp
	accounthandlerchooserdialog.cpp
	util.cpp
	simpledialog.cpp
	servicediscoverywidget.cpp
	eventsnotifier.cpp
	zoomeventfilter.cpp
	drawattentiondialog.cpp
	consolewidget.cpp
	activitydialog.cpp
	mooddialog.cpp
	callmanager.cpp
	clmodel.cpp
	callchatwidget.cpp
	chattabwebview.cpp
	locationdialog.cpp
	msgformatterwidget.cpp
	acceptriexdialog.cpp
	shareriexdialog.cpp
	searchwidget.cpp
	mucinvitedialog.cpp
	groupsenddialog.cpp
	actionsmanager.cpp
	mediadevicemanager.cpp
	iodevicesink.cpp
	iodevicesrc.cpp
	)
SET (HEADERS
	azoth.h
	core.h
	mainwidget.h
	chattabsmanager.h
	chattab.h
	sortfilterproxymodel.h
	accountslistwidget.h
	setstatusdialog.h
	pluginmanager.h
	proxyobject.h
	textedit.h
	xmlsettingsmanager.h
	contactlistdelegate.h
	addcontactdialog.h
	joinconferencedialog.h
	groupeditordialog.h
	transferjobmanager.h
	addaccountwizardfirstpage.h
	bookmarksmanagerdialog.h
	accounthandlerchooserdialog.h
	util.h
	simpledialog.h
	servicediscoverywidget.h
	eventsnotifier.h
	zoomeventfilter.h
	drawattentiondialog.h
	consolewidget.h
	activitydialog.h
	mooddialog.h
	callmanager.h
	clmodel.h
	callchatwidget.h
	chattabwebview.h
	locationdialog.h
	msgformatterwidget.h
	acceptriexdialog.h
	shareriexdialog.h
	searchwidget.h
	mucinvitedialog.h
	groupsenddialog.h
	actionsmanager.h
	mediadevicemanager.h
	iodevicesink.h
	iodevicesrc.h
	animatediconmanager.h
	sourcetrackingmodel.h
	interfaces/iprotocolplugin.h
	interfaces/iaccount.h
	interfaces/iprotocol.h
	interfaces/iclentry.h
	)
SET (FORMS
	mainwidget.ui
	chattab.ui
	accountslistwidget.ui
	setstatusdialog.ui
	addcontactdialog.ui
	joinconferencedialog.ui
	groupeditordialog.ui
	addaccountwizardfirstpage.ui
	bookmarksmanagerdialog.ui
	accounthandlerchooserdialog.ui
	simpledialog.ui
	servicediscoverywidget.ui
	drawattentiondialog.ui
	consolewidget.ui
	activitydialog.ui
	mooddialog.ui
	callchatwidget.ui
	locationdialog.ui
	acceptriexdialog.ui
	shareriexdialog.ui
	searchwidget.ui
	mucinvitedialog.ui
	groupsenddialog.ui
	)
SET (RESOURCES azothresources.qrc)

IF (ENABLE_CRYPT)
	SET (HEADERS ${HEADERS} pgpkeyselectiondialog.h)
	SET (SRCS ${SRCS} pgpkeyselectiondialog.cpp)
	SET (FORMS ${FORMS} pgpkeyselectiondialog.ui)
ENDIF (ENABLE_CRYPT)

CreateTrVars ("azoth" "en;es;ru_RU;uk_UA" TRANSLATIONS COMPILED_TRANSLATIONS)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

IF (WIN32)
	FIND_PACKAGE (Speex REQUIRED)
	SET (CUSTOM_SPEEX ${SPEEX_LIBRARIES})
ENDIF (WIN32)

ADD_CUSTOM_COMMAND (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QCA2_LIBRARIES}
	${CUSTOM_SPEEX}
	${QTGSTREAMER_LIBRARY}
	${QTGSTREAMER_UTILS_LIBRARY}
	)
INSTALL (TARGETS leechcraft_azoth DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES azothsettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/azoth DESTINATION share/leechcraft)

SET (AZOTH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

OPTION (ENABLE_AZOTH_ACETAMIDE "Build Acetamide, IRC support for Azoth" ON)
OPTION (ENABLE_AZOTH_ADIUMSTYLES "Build support for Adium styles" ON)
OPTION (ENABLE_AZOTH_AUTOPASTE "Build Autopaste for automatic pasting of long messages to pastebins" ON)
OPTION (ENABLE_AZOTH_AUTOIDLER "Build Autoidler for automatic changing of status according to idle time" ON)
OPTION (ENABLE_AZOTH_CHATHISTORY "Build ChatHistory which keeps, well, chat history" ON)
OPTION (ENABLE_AZOTH_DEPESTER "Build Depester for ignoring unwanted MUC participants" ON)
OPTION (ENABLE_AZOTH_EMBEDMEDIA "Build EmbedMedia  enables embedding different media objects in chat tab" ON)
OPTION (ENABLE_AZOTH_HERBICIDE "Build Herbicide, a basic antispam plugin" ON)
OPTION (ENABLE_AZOTH_HILI "Build HiLi, plugin for customizing conference highlights" ON)
OPTION (ENABLE_AZOTH_ISTERIQUE "Build Isterique, plugin for decapitalizing all-caps text" ON)
OPTION (ENABLE_AZOTH_JUICK "Build plugin for the juick.com microblogging service" ON)
OPTION (ENABLE_AZOTH_LASTSEEN "Build plugin for keeping track of the date of contacts' being online" ON)
OPTION (ENABLE_AZOTH_METACONTACTS "Build support for metacontacts" ON)
OPTION (ENABLE_AZOTH_MODNOK "Build Modnok for rendering and displaying LaTeX formulas" ON)
OPTION (ENABLE_AZOTH_NATIVEEMOTICONS "Build support for Azoth's own emoticons packs" ON)
OPTION (ENABLE_AZOTH_OTROID "Build OTRoid for supporting Off-the-Record messaging" OFF)
OPTION (ENABLE_AZOTH_P100Q "Build plugin for the psto.net microblogging service" ON)
OPTION (ENABLE_AZOTH_ROSENTHAL "Build Rosenthal, spellchecker plugin for Azoth" ON)
OPTION (ENABLE_AZOTH_STANDARDSTYLES "Build support for standard Azoth styles" ON)
OPTION (ENABLE_AZOTH_XOOX "Build Xoox, the QXmpp-aided XMPP support for Azoth" ON)
OPTION (ENABLE_AZOTH_XTAZY "Build Xtazy for publishing current user tune" ON)

IF (ENABLE_AZOTH_ACETAMIDE)
	ADD_SUBDIRECTORY (plugins/acetamide)
ENDIF (ENABLE_AZOTH_ACETAMIDE)
IF (ENABLE_AZOTH_ADIUMSTYLES)
	ADD_SUBDIRECTORY (plugins/adiumstyles)
ENDIF (ENABLE_AZOTH_ADIUMSTYLES)
IF (ENABLE_AZOTH_AUTOPASTE)
	ADD_SUBDIRECTORY (plugins/autopaste)
ENDIF (ENABLE_AZOTH_AUTOPASTE)
IF (ENABLE_AZOTH_AUTOIDLER)
	ADD_SUBDIRECTORY (plugins/autoidler)
ENDIF (ENABLE_AZOTH_AUTOIDLER)
IF (ENABLE_AZOTH_CHATHISTORY)
	ADD_SUBDIRECTORY (plugins/chathistory)
ENDIF (ENABLE_AZOTH_CHATHISTORY)
IF (ENABLE_AZOTH_DEPESTER)
	ADD_SUBDIRECTORY (plugins/depester)
ENDIF (ENABLE_AZOTH_DEPESTER)
IF (ENABLE_AZOTH_EMBEDMEDIA)
    ADD_SUBDIRECTORY (plugins/embedmedia)
ENDIF (ENABLE_AZOTH_EMBEDMEDIA)
IF (ENABLE_AZOTH_HERBICIDE)
	ADD_SUBDIRECTORY (plugins/herbicide)
ENDIF (ENABLE_AZOTH_HERBICIDE)
IF (ENABLE_AZOTH_HILI)
	ADD_SUBDIRECTORY (plugins/hili)
ENDIF (ENABLE_AZOTH_HILI)
IF (ENABLE_AZOTH_ISTERIQUE)
	ADD_SUBDIRECTORY (plugins/isterique)
ENDIF (ENABLE_AZOTH_ISTERIQUE)
IF (ENABLE_AZOTH_JUICK)
	ADD_SUBDIRECTORY (plugins/juick)
ENDIF (ENABLE_AZOTH_JUICK)
IF (ENABLE_AZOTH_LASTSEEN)
	ADD_SUBDIRECTORY (plugins/lastseen)
ENDIF (ENABLE_AZOTH_LASTSEEN)
IF (ENABLE_AZOTH_METACONTACTS)
	ADD_SUBDIRECTORY (plugins/metacontacts)
ENDIF (ENABLE_AZOTH_METACONTACTS)
IF (ENABLE_AZOTH_MODNOK)
	ADD_SUBDIRECTORY (plugins/modnok)
ENDIF (ENABLE_AZOTH_MODNOK)
IF (ENABLE_AZOTH_NATIVEEMOTICONS)
	ADD_SUBDIRECTORY (plugins/nativeemoticons)
ENDIF (ENABLE_AZOTH_NATIVEEMOTICONS)
IF (ENABLE_AZOTH_OTROID)
	ADD_SUBDIRECTORY (plugins/otroid)
ENDIF (ENABLE_AZOTH_OTROID)
IF (ENABLE_AZOTH_P100Q)
	ADD_SUBDIRECTORY (plugins/p100q)
ENDIF (ENABLE_AZOTH_P100Q)
IF (ENABLE_AZOTH_ROSENTHAL)
	ADD_SUBDIRECTORY (plugins/rosenthal)
ENDIF (ENABLE_AZOTH_ROSENTHAL)
IF (ENABLE_AZOTH_STANDARDSTYLES)
	ADD_SUBDIRECTORY (plugins/standardstyles)
ENDIF (ENABLE_AZOTH_STANDARDSTYLES)
IF (ENABLE_AZOTH_XOOX)
	ADD_SUBDIRECTORY (plugins/xoox)
ENDIF (ENABLE_AZOTH_XOOX)
IF (ENABLE_AZOTH_XTAZY)
	ADD_SUBDIRECTORY (plugins/xtazy)
ENDIF (ENABLE_AZOTH_XTAZY)
