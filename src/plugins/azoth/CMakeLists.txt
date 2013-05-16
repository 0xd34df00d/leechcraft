CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth)
INCLUDE (InitLCPlugin OPTIONAL)
SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

OPTION (ENABLE_MEDIACALLS "Enable support for media calls" ON)

IF (NOT WIN32)
	SET (QT_USE_QTDBUS TRUE)
ENDIF (NOT WIN32)
SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTWEBKIT TRUE)
SET (QT_USE_QTSQL TRUE)
SET (QT_USE_QTXML TRUE)

IF (ENABLE_MEDIACALLS)
	SET (QT_USE_QTMULTIMEDIA TRUE)
ENDIF (ENABLE_MEDIACALLS)

INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

OPTION (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
OPTION (WITH_DOCS_INSTALL "Install generated documentation (if WITH_DOCS is set)" OFF)
IF (WITH_DOCS)
	FIND_PACKAGE (Doxygen REQUIRED)

	SET (DOXYDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../doc/doxygen/azoth/")
	SET (DOXYFILE "${DOXYDIR}/Doxyfile")

	ADD_CUSTOM_TARGET (doc_azoth ALL
		"sed" "-i" "s/^PROJECT_NUMBER.*/PROJECT_NUMBER = '${LEECHCRAFT_VERSION}'/" "${DOXYFILE}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
		COMMAND "mv" "-f" "${DOXYDIR}/out" "${CMAKE_CURRENT_BINARY_DIR}/out"
		WORKING_DIRECTORY "${DOXYDIR}"
	)
	IF (WITH_DOCS_INSTALL)
		INSTALL (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-azoth-${LEECHCRAFT_VERSION}")
	ENDIF (WITH_DOCS_INSTALL)
ENDIF (WITH_DOCS)

OPTION (ENABLE_QTMULTIMEDIA_KLUDGE "Enable kludges for finding QtMultimedia" OFF)

IF (ENABLE_MEDIACALLS)
	IF ((${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 2.8 AND ${CMAKE_PATCH_VERSION} LESS 4) OR ENABLE_QTMULTIMEDIA_KLUDGE)
		IF (${QT_VERSION_MINOR} EQUAL 6)
			INCLUDE_DIRECTORIES (/usr/include/qt4/QtMultimedia)
			SET (QTMULTIMEDIA_KLUDGE "QtMultimedia")
		ELSE (${QT_VERSION_MINOR} EQUAL 6)
			INCLUDE_DIRECTORIES (/usr/include/QtMultimediaKit)
			INCLUDE_DIRECTORIES (/usr/include/QtMobility)
			SET (QTMULTIMEDIA_KLUDGE "QtMultimediaKit")
		ENDIF (${QT_VERSION_MINOR} EQUAL 6)
	ENDIF ((${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} EQUAL 2.8 AND ${CMAKE_PATCH_VERSION} LESS 4) OR ENABLE_QTMULTIMEDIA_KLUDGE)
ENDIF (ENABLE_MEDIACALLS)

OPTION (ENABLE_CRYPT "Enable QCA2-based support for PGP" ON)

IF (ENABLE_CRYPT)
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

IF (ENABLE_MEDIACALLS)
	ADD_DEFINITIONS (-DENABLE_MEDIACALLS)
ENDIF (ENABLE_MEDIACALLS)

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
	importmanager.cpp
	accountactionsmanager.cpp
	unreadqueuemanager.cpp
	chatstyleoptionmanager.cpp
	microblogstab.cpp
	riexhandler.cpp
	filesenddialog.cpp
	pendinguploadpaster.cpp
	contactdropfilter.cpp
	colorlisteditorwidget.cpp
	customstatusesmanager.cpp
	userslistwidget.cpp
	keyboardrosterfixer.cpp
	statuschangemenumanager.cpp
	customchatstylemanager.cpp
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
	microblogstab.ui
	filesenddialog.ui
	colorlisteditorwidget.ui
	userslistwidget.ui
	)
SET (RESOURCES azothresources.qrc)

IF (ENABLE_CRYPT)
	SET (SRCS ${SRCS} pgpkeyselectiondialog.cpp)
	SET (FORMS ${FORMS} pgpkeyselectiondialog.ui)
ENDIF (ENABLE_CRYPT)

CreateTrs ("azoth" "en;es;ru_RU;tt;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("azoth" "en;ru_RU" "${SRCS}" "${FORMS}" "azothsettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

IF (WIN32)
	FIND_PACKAGE (Speex REQUIRED)
	SET (CUSTOM_SPEEX ${SPEEX_LIBRARIES})
ENDIF (WIN32)

ADD_LIBRARY (leechcraft_azoth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QTMULTIMEDIA_KLUDGE}
	${QCA2_LIBRARIES}
	${CUSTOM_SPEEX}
	)
INSTALL (TARGETS leechcraft_azoth DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES azothsettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})
INSTALL (DIRECTORY interfaces DESTINATION include/leechcraft)

SET (AZOTH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

OPTION (ENABLE_AZOTH_ACETAMIDE "Build Acetamide, IRC support for Azoth" ON)
OPTION (ENABLE_AZOTH_ADIUMSTYLES "Build support for Adium styles" ON)
OPTION (ENABLE_AZOTH_ASTRALITY "Build support for protocols provided by Telepathy" OFF)
OPTION (ENABLE_AZOTH_AUTOPASTE "Build Autopaste for automatic pasting of long messages to pastebins" ON)
OPTION (ENABLE_AZOTH_AUTOIDLER "Build Autoidler for automatic changing of status according to idle time" ON)
OPTION (ENABLE_AZOTH_BIRTHDAYNOTIFIER "Build Birthday Notifier so you don't miss your contacts' birthdays" ON)
OPTION (ENABLE_AZOTH_CHATHISTORY "Build ChatHistory which keeps, well, chat history" ON)
OPTION (ENABLE_AZOTH_DEPESTER "Build Depester for ignoring unwanted MUC participants" ON)
OPTION (ENABLE_AZOTH_EMBEDMEDIA "Build EmbedMedia  enables embedding different media objects in chat tab" ON)
OPTION (ENABLE_AZOTH_HERBICIDE "Build Herbicide, a basic antispam plugin" ON)
OPTION (ENABLE_AZOTH_HILI "Build HiLi, plugin for customizing conference highlights" ON)
OPTION (ENABLE_AZOTH_ISTERIQUE "Build Isterique, plugin for decapitalizing all-caps text" ON)
OPTION (ENABLE_AZOTH_JUICK "Build plugin for the juick.com microblogging service" ON)
OPTION (ENABLE_AZOTH_KEESO "Build Keeso for TrAnSfOrMiNg TeXt LiKe ThIs" ON)
OPTION (ENABLE_AZOTH_LASTSEEN "Build plugin for keeping track of the date of contacts' being online" ON)
OPTION (ENABLE_AZOTH_METACONTACTS "Build support for metacontacts" ON)
OPTION (ENABLE_AZOTH_MODNOK "Build Modnok for rendering and displaying LaTeX formulas" ON)
OPTION (ENABLE_AZOTH_NATIVEEMOTICONS "Build support for Azoth's own emoticons packs" ON)
OPTION (ENABLE_AZOTH_OTROID "Build OTRoid for supporting Off-the-Record messaging" OFF)
OPTION (ENABLE_AZOTH_P100Q "Build plugin for the psto.net microblogging service" ON)
OPTION (ENABLE_AZOTH_ROSENTHAL "Build Rosenthal, spellchecker plugin for Azoth" ON)
OPTION (ENABLE_AZOTH_SHX "Build ShX, shell command runner" ON)
OPTION (ENABLE_AZOTH_STANDARDSTYLES "Build support for standard Azoth styles" ON)
OPTION (ENABLE_AZOTH_VADER "Build support for the MRIM protocol" ON)
OPTION (ENABLE_AZOTH_VELVETBIRD "Build libpurple support plugin" OFF)
OPTION (ENABLE_AZOTH_XOOX "Build Xoox, the QXmpp-aided XMPP support for Azoth" ON)
OPTION (ENABLE_AZOTH_XTAZY "Build Xtazy for publishing current user tune" ON)
OPTION (ENABLE_AZOTH_ZHEET "Build Zheet, the libmsn-based Windows Live support for Azoth" OFF)

IF (ENABLE_AZOTH_ACETAMIDE)
	ADD_SUBDIRECTORY (plugins/acetamide)
ENDIF (ENABLE_AZOTH_ACETAMIDE)
IF (ENABLE_AZOTH_ADIUMSTYLES)
	ADD_SUBDIRECTORY (plugins/adiumstyles)
ENDIF (ENABLE_AZOTH_ADIUMSTYLES)
IF (ENABLE_AZOTH_ASTRALITY)
	ADD_SUBDIRECTORY (plugins/astrality)
ENDIF (ENABLE_AZOTH_ASTRALITY)
IF (ENABLE_AZOTH_AUTOPASTE)
	ADD_SUBDIRECTORY (plugins/autopaste)
ENDIF (ENABLE_AZOTH_AUTOPASTE)
IF (ENABLE_AZOTH_AUTOIDLER)
	ADD_SUBDIRECTORY (plugins/autoidler)
ENDIF (ENABLE_AZOTH_AUTOIDLER)
IF (ENABLE_AZOTH_BIRTHDAYNOTIFIER)
	ADD_SUBDIRECTORY (plugins/birthdaynotifier)
ENDIF (ENABLE_AZOTH_BIRTHDAYNOTIFIER)
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
IF (ENABLE_AZOTH_KEESO)
	ADD_SUBDIRECTORY (plugins/keeso)
ENDIF (ENABLE_AZOTH_KEESO)
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
IF (ENABLE_AZOTH_SHX)
	ADD_SUBDIRECTORY (plugins/shx)
ENDIF (ENABLE_AZOTH_SHX)
IF (ENABLE_AZOTH_STANDARDSTYLES)
	ADD_SUBDIRECTORY (plugins/standardstyles)
ENDIF (ENABLE_AZOTH_STANDARDSTYLES)
IF (ENABLE_AZOTH_VADER)
	ADD_SUBDIRECTORY (plugins/vader)
ENDIF (ENABLE_AZOTH_VADER)
IF (ENABLE_AZOTH_VELVETBIRD)
	ADD_SUBDIRECTORY (plugins/velvetbird)
ENDIF (ENABLE_AZOTH_VELVETBIRD)
IF (ENABLE_AZOTH_XOOX)
	ADD_SUBDIRECTORY (plugins/xoox)
ENDIF (ENABLE_AZOTH_XOOX)
IF (ENABLE_AZOTH_XTAZY)
	ADD_SUBDIRECTORY (plugins/xtazy)
ENDIF (ENABLE_AZOTH_XTAZY)
IF (ENABLE_AZOTH_ZHEET)
	ADD_SUBDIRECTORY (plugins/zheet)
ENDIF (ENABLE_AZOTH_ZHEET)
