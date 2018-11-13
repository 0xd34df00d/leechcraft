cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

include (CMakeDependentOption)

option (ENABLE_MEDIACALLS "Enable support for media calls" ON)

option (USE_AZOTH_BOOST_LOCALE "Use Boost.Locale for better datetime formatting in Azoth" ON)
if (USE_AZOTH_BOOST_LOCALE)
	find_Package (Boost REQUIRED COMPONENTS locale)

	add_definitions (-DUSE_BOOST_LOCALE)
endif ()

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

option (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
option (WITH_DOCS_INSTALL "Install generated documentation (if WITH_DOCS is set)" OFF)
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
	if (WITH_DOCS_INSTALL)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-azoth-${LEECHCRAFT_VERSION}")
	endif ()
endif ()

if (ENABLE_MEDIACALLS)
	add_definitions (-DENABLE_MEDIACALLS)
endif ()

option (ENABLE_CRYPT "Enable QCA2-based support for PGP" ON)

set (QCA2_INCLUDE_DIR)
set (QCA2_LIBRARIES)

if (ENABLE_CRYPT)
	find_package (QCA2 REQUIRED)
	include_directories (${QCA2_INCLUDE_DIR})

	add_definitions (-DENABLE_CRYPT)
endif ()

set (COMMON_SRCS
	azothcommon.cpp
	)
add_library (leechcraft_azoth_common STATIC
	${COMMON_SRCS}
	)
set_target_properties (leechcraft_azoth_common PROPERTIES POSITION_INDEPENDENT_CODE True)
FindQtLibs (leechcraft_azoth_common Core)

set (SRCS
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
	)
set (FORMS
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
	advancedpermchangedialog.ui
	groupremovedialog.ui
	serverhistorywidget.ui
	bookmarkeditdialog.ui
	sslerrorsdialog.ui
	)
set (RESOURCES azothresources.qrc)

if (ENABLE_CRYPT)
	set (SRCS ${SRCS} pgpkeyselectiondialog.cpp cryptomanager.cpp)
	set (FORMS ${FORMS} pgpkeyselectiondialog.ui)
endif ()

CreateTrs ("azoth" "en;es;ru_RU;tt;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("azoth" "en;ru_RU" "${SRCS}" "${FORMS}" "azothsettings.xml")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

if (WIN32)
	find_package (Speex REQUIRED)
	set (CUSTOM_SPEEX ${SPEEX_LIBRARIES})
endif ()

add_library (leechcraft_azoth SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_azoth
	${Boost_LOCALE_LIBRARY}
	${LEECHCRAFT_LIBRARIES}
	${QCA2_LIBRARIES}
	${CUSTOM_SPEEX}
	leechcraft_azoth_common
	)
install (TARGETS leechcraft_azoth DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/azoth DESTINATION ${LC_SHARE_DEST})
install (DIRECTORY interfaces DESTINATION include/leechcraft)

FindQtLibs (leechcraft_azoth Concurrent Network Sql WebKitWidgets Widgets Xml)
if (ENABLE_MEDIACALLS)
	FindQtLibs (leechcraft_azoth Multimedia)
endif ()

set (AZOTH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

option (ENABLE_AZOTH_ABBREV "Build Abbrev for supporting abbreviations" ON)
option (ENABLE_AZOTH_ACETAMIDE "Build Acetamide, IRC support for Azoth" ON)
option (ENABLE_AZOTH_ADIUMSTYLES "Build support for Adium styles" ON)
option (ENABLE_AZOTH_ASTRALITY "Build support for protocols provided by Telepathy" OFF)
option (ENABLE_AZOTH_AUTOPASTE "Build Autopaste for automatic pasting of long messages to pastebins" ON)
option (ENABLE_AZOTH_AUTOIDLER "Build Autoidler for automatic changing of status according to idle time" ON)
option (ENABLE_AZOTH_BIRTHDAYNOTIFIER "Build Birthday Notifier so you don't miss your contacts' birthdays" ON)
option (ENABLE_AZOTH_CHATHISTORY "Build ChatHistory which keeps, well, chat history" ON)
option (ENABLE_AZOTH_DEPESTER "Build Depester for ignoring unwanted MUC participants" ON)
option (ENABLE_AZOTH_EMBEDMEDIA "Build EmbedMedia  enables embedding different media objects in chat tab" ON)
option (ENABLE_AZOTH_HERBICIDE "Build Herbicide, a basic antispam plugin" ON)
option (ENABLE_AZOTH_HILI "Build HiLi, plugin for customizing conference highlights" ON)
option (ENABLE_AZOTH_ISTERIQUE "Build Isterique, plugin for decapitalizing all-caps text" ON)
option (ENABLE_AZOTH_JUICK "Build plugin for the juick.com microblogging service" ON)
option (ENABLE_AZOTH_KEESO "Build Keeso for TrAnSfOrMiNg TeXt LiKe ThIs" ON)
option (ENABLE_AZOTH_LASTSEEN "Build plugin for keeping track of the date of contacts' being online" ON)
option (ENABLE_AZOTH_METACONTACTS "Build support for metacontacts" ON)
option (ENABLE_AZOTH_MODNOK "Build Modnok for rendering and displaying LaTeX formulas" ON)
option (ENABLE_AZOTH_MUCOMMANDS "Build Mucommands, the module providing some common conference-oriented commands" ON)
option (ENABLE_AZOTH_MURM "Build Murm, special plugin for extensive VKontakte messaging support" ON)
option (ENABLE_AZOTH_NATIVEEMOTICONS "Build support for Azoth's own emoticons packs" ON)
option (ENABLE_AZOTH_OTROID "Build OTRoid for supporting Off-the-Record messaging" OFF)
option (ENABLE_AZOTH_ROSENTHAL "Build Rosenthal, spellchecker plugin for Azoth" ON)
option (ENABLE_AZOTH_SARIN "Build Sarin, Tox support for Azoth" OFF)
option (ENABLE_AZOTH_SHX "Build ShX, shell command runner" ON)
option (ENABLE_AZOTH_STANDARDSTYLES "Build support for standard Azoth styles" ON)
option (ENABLE_AZOTH_TRACOLOR "Build support for color-indicating contacts activity tracker" ON)
option (ENABLE_AZOTH_VADER "Build support for the MRIM protocol" ON)
option (ENABLE_AZOTH_VELVETBIRD "Build libpurple support plugin" OFF)
option (ENABLE_AZOTH_XOOX "Build Xoox, the QXmpp-aided XMPP support for Azoth" ON)
option (ENABLE_AZOTH_XTAZY "Build Xtazy for publishing current user tune" ON)
option (ENABLE_AZOTH_ZHEET "Build Zheet, the libmsn-based Windows Live support for Azoth" OFF)
option (ENABLE_AZOTH_WOODPECKER "Build Woodpecker, twitter client" ON)

if (ENABLE_AZOTH_ABBREV)
	add_subdirectory (plugins/abbrev)
endif ()
if (ENABLE_AZOTH_ACETAMIDE)
	add_subdirectory (plugins/acetamide)
endif ()
if (ENABLE_AZOTH_ADIUMSTYLES)
	add_subdirectory (plugins/adiumstyles)
endif ()
if (ENABLE_AZOTH_ASTRALITY)
	add_subdirectory (plugins/astrality)
endif ()
if (ENABLE_AZOTH_AUTOPASTE)
	add_subdirectory (plugins/autopaste)
endif ()
if (ENABLE_AZOTH_AUTOIDLER)
	add_subdirectory (plugins/autoidler)
endif ()
if (ENABLE_AZOTH_BIRTHDAYNOTIFIER)
	add_subdirectory (plugins/birthdaynotifier)
endif ()
if (ENABLE_AZOTH_CHATHISTORY)
	add_subdirectory (plugins/chathistory)
endif ()
if (ENABLE_AZOTH_DEPESTER)
	add_subdirectory (plugins/depester)
endif ()
if (ENABLE_AZOTH_EMBEDMEDIA)
    add_subdirectory (plugins/embedmedia)
endif ()
if (ENABLE_AZOTH_HERBICIDE)
	add_subdirectory (plugins/herbicide)
endif ()
if (ENABLE_AZOTH_HILI)
	add_subdirectory (plugins/hili)
endif ()
if (ENABLE_AZOTH_ISTERIQUE)
	add_subdirectory (plugins/isterique)
endif ()
if (ENABLE_AZOTH_JUICK)
	add_subdirectory (plugins/juick)
endif ()
if (ENABLE_AZOTH_KEESO)
	add_subdirectory (plugins/keeso)
endif ()
if (ENABLE_AZOTH_LASTSEEN)
	add_subdirectory (plugins/lastseen)
endif ()
if (ENABLE_AZOTH_METACONTACTS)
	add_subdirectory (plugins/metacontacts)
endif ()
if (ENABLE_AZOTH_MODNOK)
	add_subdirectory (plugins/modnok)
endif ()
if (ENABLE_AZOTH_MUCOMMANDS)
	add_subdirectory (plugins/mucommands)
endif ()
if (ENABLE_AZOTH_MURM)
	add_subdirectory (plugins/murm)
endif ()
if (ENABLE_AZOTH_NATIVEEMOTICONS)
	add_subdirectory (plugins/nativeemoticons)
endif ()
if (ENABLE_AZOTH_OTROID)
	add_subdirectory (plugins/otroid)
endif ()
if (ENABLE_AZOTH_ROSENTHAL)
	add_subdirectory (plugins/rosenthal)
endif ()
if (ENABLE_AZOTH_SARIN)
	add_subdirectory (plugins/sarin)
endif ()
if (ENABLE_AZOTH_SHX)
	add_subdirectory (plugins/shx)
endif ()
if (ENABLE_AZOTH_STANDARDSTYLES)
	add_subdirectory (plugins/standardstyles)
endif ()
if (ENABLE_AZOTH_TRACOLOR)
	add_subdirectory (plugins/tracolor)
endif ()
if (ENABLE_AZOTH_VADER)
	add_subdirectory (plugins/vader)
endif ()
if (ENABLE_AZOTH_VELVETBIRD)
	add_subdirectory (plugins/velvetbird)
endif ()
if (ENABLE_AZOTH_XOOX)
	add_subdirectory (plugins/xoox)
endif ()
if (ENABLE_AZOTH_XTAZY)
	add_subdirectory (plugins/xtazy)
endif ()
if (ENABLE_AZOTH_ZHEET)
	add_subdirectory (plugins/zheet)
endif ()
if (ENABLE_AZOTH_WOODPECKER)
	add_subdirectory (plugins/woodpecker)
endif ()

