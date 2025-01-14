cmake_minimum_required (VERSION 3.10)
project (azoth)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_MEDIACALLS "Enable support for media calls" ON)

option (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
if (WITH_DOCS)
	find_package (Doxygen REQUIRED)

	set (DOXYDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../doc/doxygen/azoth/")
	set (DOXYFILE "${DOXYDIR}/Doxyfile")

	add_custom_target (doc_azoth ALL
		"sed" "-i" "s/^PROJECT_NUMBER.*/PROJECT_NUMBER = '${LEECHCRAFT_VERSION}'/" "${DOXYFILE}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
		COMMAND "mv" "-f" "${DOXYDIR}/out" "${CMAKE_CURRENT_BINARY_DIR}/out"
		WORKING_DIRECTORY "${DOXYDIR}"
	)
endif ()

if (ENABLE_MEDIACALLS)
	add_definitions (-DENABLE_MEDIACALLS)
	set (EXTRA_MULTIMEDIA_LIBRARY Multimedia)
endif ()

option (ENABLE_CRYPT "Enable QCA2-based support for PGP" ON)

if (ENABLE_CRYPT)
	find_package(Qca-qt${LC_QT_VERSION} REQUIRED)

	add_definitions (-DENABLE_CRYPT)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		components/dialogs/acceptriexdialog.cpp
		components/dialogs/accounthandlerchooserdialog.cpp
		components/dialogs/activitydialog.cpp
		components/dialogs/addcontactdialog.cpp
		components/dialogs/advancedpermchangedialog.cpp
		components/dialogs/bookmarkeditdialog.cpp
		components/dialogs/bookmarksmanagerdialog.cpp
		components/dialogs/drawattentiondialog.cpp
		components/dialogs/filesenddialog.cpp
		components/dialogs/groupeditordialog.cpp
		components/dialogs/groupremovedialog.cpp
		components/dialogs/groupsenddialog.cpp
		components/dialogs/joinconferencedialog.cpp
		components/dialogs/locationdialog.cpp
		components/dialogs/mooddialog.cpp
		components/dialogs/mucinvitedialog.cpp
		components/dialogs/pgpkeyselectiondialog.cpp
		components/dialogs/setstatusdialog.cpp
		components/dialogs/shareriexdialog.cpp
		components/dialogs/simpledialog.cpp
		components/dialogs/sslerrorsdialog.cpp
		components/dialogs/userslistwidget.cpp
		$<$<BOOL:${ENABLE_CRYPT}>:components/dialogs/pgpkeyselectiondialog.cpp>
		components/roster/clmodel.cpp
		components/roster/cltooltipmanager.cpp
		components/roster/contactlistdelegate.cpp
		components/roster/contactslistview.cpp
		components/roster/expansionstatemanager.cpp
		components/roster/keyboardrosterfixer.cpp
		components/roster/mainwidget.cpp
		components/roster/sortfilterproxymodel.cpp
		azoth.cpp
		azothschemehandler.cpp
		core.cpp
		chattabsmanager.cpp
		chattab.cpp
		accountslistwidget.cpp
		pluginmanager.cpp
		proxyobject.cpp
		textedit.cpp
		transferjobmanager.cpp
		addaccountwizardfirstpage.cpp
		util.cpp
		servicediscoverywidget.cpp
		consolewidget.cpp
		callmanager.cpp
		callchatwidget.cpp
		chattabwebview.cpp
		msgformatterwidget.cpp
		searchwidget.cpp
		actionsmanager.cpp
		importmanager.cpp
		accountactions.cpp
		unreadqueuemanager.cpp
		chatstyleoptionmanager.cpp
		microblogstab.cpp
		riexhandler.cpp
		pendinguploadpaster.cpp
		contactdropfilter.cpp
		colorlisteditorwidget.cpp
		customstatusesmanager.cpp
		statuschange.cpp
		customchatstylemanager.cpp
		coremessage.cpp
		dummymsgmanager.cpp
		serverhistorywidget.cpp
		dndutil.cpp
		corecommandsmanager.cpp
		resourcesmanager.cpp
		notificationsmanager.cpp
		msgeditautocompleter.cpp
		msgsender.cpp
		avatarsmanager.cpp
		avatarsstorage.cpp
		avatarsstorageondisk.cpp
		avatarsstoragethread.cpp
		historysyncer.cpp
		chattabpartstatemanager.cpp
		sslerrorshandler.cpp
		sslerrorschoicestorage.cpp
		tabbase.cpp
		categoryactions.cpp
		$<$<BOOL:${ENABLE_CRYPT}>:cryptomanager.cpp>
	SETTINGS azothsettings.xml
	RESOURCES azothresources.qrc
	QT_COMPONENTS Concurrent Network Sql WebEngineWidgets Widgets Xml ${EXTRA_MULTIMEDIA_LIBRARY}
	LINK_LIBRARIES $<$<BOOL:${ENABLE_CRYPT}>:qca-qt${LC_QT_VERSION}>
	INSTALL_SHARE
	)

install (DIRECTORY interfaces DESTINATION include/leechcraft)

SUBPLUGIN (ABBREV "Build Abbrev for supporting abbreviations")
SUBPLUGIN (ACETAMIDE "Build Acetamide, IRC support for Azoth")
SUBPLUGIN (ADIUMSTYLES "Build support for Adium styles")
SUBPLUGIN (ASTRALITY "Build support for protocols provided by Telepathy" OFF)
SUBPLUGIN (AUTOPASTE "Build Autopaste for automatic pasting of long messages to pastebins")
SUBPLUGIN (AUTOIDLER "Build Autoidler for automatic changing of status according to idle time")
SUBPLUGIN (BIRTHDAYNOTIFIER "Build Birthday Notifier so you don't miss your contacts' birthdays")
SUBPLUGIN (CHATHISTORY "Build ChatHistory which keeps, well, chat history")
SUBPLUGIN (DEPESTER "Build Depester for ignoring unwanted MUC participants")
SUBPLUGIN (EMBEDMEDIA "Build EmbedMedia  enables embedding different media objects in chat tab")
SUBPLUGIN (HERBICIDE "Build Herbicide, a basic antispam plugin")
SUBPLUGIN (HILI "Build HiLi, plugin for customizing conference highlights")
SUBPLUGIN (ISTERIQUE "Build Isterique, plugin for decapitalizing all-caps text")
SUBPLUGIN (KEESO "Build Keeso for TrAnSfOrMiNg TeXt LiKe ThIs")
SUBPLUGIN (LASTSEEN "Build plugin for keeping track of the date of contacts' being online")
SUBPLUGIN (METACONTACTS "Build support for metacontacts")
SUBPLUGIN (MODNOK "Build Modnok for rendering and displaying LaTeX formulas")
SUBPLUGIN (MUCOMMANDS "Build Mucommands, the module providing some common conference-oriented commands")
SUBPLUGIN (MURM "Build Murm, special plugin for extensive VKontakte messaging support")
SUBPLUGIN (NATIVEEMOTICONS "Build support for Azoth's own emoticons packs")
SUBPLUGIN (OTROID "Build OTRoid for supporting Off-the-Record messaging" OFF)
SUBPLUGIN (ROSENTHAL "Build Rosenthal, spellchecker plugin for Azoth")
SUBPLUGIN (SARIN "Build Sarin, Tox support for Azoth" OFF)
SUBPLUGIN (SHX "Build ShX, shell command runner")
SUBPLUGIN (STANDARDSTYLES "Build support for standard Azoth styles")
SUBPLUGIN (TRACOLOR "Build support for color-indicating contacts activity tracker")
SUBPLUGIN (VELVETBIRD "Build libpurple support plugin" OFF)
SUBPLUGIN (XOOX "Build Xoox, the QXmpp-aided XMPP support for Azoth")
SUBPLUGIN (XTAZY "Build Xtazy for publishing current user tune")
