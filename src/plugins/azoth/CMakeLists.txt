cmake_minimum_required (VERSION 3.10)
project (leechcraft_azoth)
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
	find_package(PkgConfig)
	pkg_check_modules(QCA2 REQUIRED IMPORTED_TARGET qca2-qt5)

	add_definitions (-DENABLE_CRYPT)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
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
		advancedpermchangedialog.cpp
		groupremovedialog.cpp
		coremessage.cpp
		dummymsgmanager.cpp
		serverhistorywidget.cpp
		dndutil.cpp
		cltooltipmanager.cpp
		corecommandsmanager.cpp
		resourcesmanager.cpp
		notificationsmanager.cpp
		msgeditautocompleter.cpp
		bookmarkeditdialog.cpp
		msgsender.cpp
		avatarsmanager.cpp
		avatarsstorage.cpp
		avatarsstorageondisk.cpp
		avatarsstoragethread.cpp
		chattabnetworkaccessmanager.cpp
		historysyncer.cpp
		chattabpartstatemanager.cpp
		sslerrorshandler.cpp
		sslerrorsdialog.cpp
		sslerrorschoicestorage.cpp
		contactslistview.cpp
		$<$<BOOL:${ENABLE_CRYPT}>:pgpkeyselectiondialog.cpp cryptomanager.cpp>
	SETTINGS azothsettings.xml
	RESOURCES azothresources.qrc
	QT_COMPONENTS Concurrent Network Sql WebKitWidgets Widgets Xml ${EXTRA_MULTIMEDIA_LIBRARY}
	LINK_LIBRARIES $<$<BOOL:${ENABLE_CRYPT}>:PkgConfig::QCA2>
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
SUBPLUGIN (JUICK "Build plugin for the juick.com microblogging service")
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
SUBPLUGIN (VADER "Build support for the MRIM protocol")
SUBPLUGIN (VELVETBIRD "Build libpurple support plugin" OFF)
SUBPLUGIN (XOOX "Build Xoox, the QXmpp-aided XMPP support for Azoth")
SUBPLUGIN (XTAZY "Build Xtazy for publishing current user tune")
