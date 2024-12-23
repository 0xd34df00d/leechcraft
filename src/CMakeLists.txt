cmake_minimum_required (VERSION 3.10)

project (leechcraft)

if (MSVC)
	set (CMAKE_CXX_FLAGS "/EHa /W3")
	set (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD /D NDEBUG")
	set (CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /GT /MDd")

	set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
	include (LibHelperWin32Macros)
endif ()
if (WIN32)
	add_definitions (-DWIN32_LEAN_AND_MEAN) # Don't include not needed headers with a lot of useless macross definitions
	add_definitions (-D_WIN32_WINNT=0x0600) # Use Windows Vista definitions. Needed by Liznoo and KInotify
	add_definitions (-DWINVER=0x0600)
	add_definitions (-DNTDDI_VERSION=0x0600)
endif ()

option (USE_ASAN "Use address sanitizer" OFF)

set (LC_QT_VERSION 5)
set (LC_LIBSUFFIX "-qt${LC_QT_VERSION}")

if (UNIX OR (WIN32 AND MINGW))
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall -Wextra")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE='QT_VERSION_CHECK(5,15,0)'")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-enum-conversion")

	if (USE_ASAN)
		add_compile_options (-fsanitize=address)
		add_link_options (-fsanitize=address)
	endif ()

	add_definitions (-DQT_MESSAGELOGCONTEXT)

	if (NOT APPLE)
		set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
		set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
	endif ()
endif ()

option (USE_UNIX_LAYOUT "Install LeechCraft with a UNIX layout on OS X" OFF)

if (APPLE AND NOT USE_UNIX_LAYOUT)
	set (CMAKE_INSTALL_RPATH "@loader_path/../Frameworks")
endif ()

if (UNIX AND (NOT APPLE OR USE_UNIX_LAYOUT))
	if (NOT LIBDIR)
		set (LIBDIR "lib${LIB_SUFFIX}")
	endif ()

	if (USE_UNIX_LAYOUT)
		add_definitions (-DUSE_UNIX_LAYOUT)
	endif ()

	set (LC_BINDIR "bin")
	set (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins-qt5")
	set (LC_TRANSLATIONS_DEST "share/leechcraft/translations")
	set (LC_SETTINGS_DEST "share/leechcraft/settings")
	set (LC_SHARE_DEST "share/leechcraft")
elseif (WIN32)
	if (NOT LIBDIR)
		set (LIBDIR "${CMAKE_INSTALL_PREFIX}")
	endif ()

	set (LC_BINDIR "${CMAKE_INSTALL_PREFIX}")
	set (LC_PLUGINS_DEST "plugins/bin")
	set (LC_TRANSLATIONS_DEST "translations")
	set (LC_SETTINGS_DEST "settings")
	set (LC_SHARE_DEST "share")
else ()
	set (LIBDIR "leechcraft.app/Contents/Frameworks")
	set (LC_BINDIR "leechcraft.app/Contents/MacOs")
	set (LC_PLUGINS_DEST "leechcraft.app/Contents/PlugIns")
	set (LC_TRANSLATIONS_DEST "leechcraft.app/Contents/Resources/translations")
	set (LC_SETTINGS_DEST "leechcraft.app/Contents/Resources/settings")
	set (LC_SHARE_DEST "leechcraft.app/Contents/Resources/share")
endif ()

find_package (Boost REQUIRED COMPONENTS date_time filesystem program_options system thread)

include (CommonLCSetup.cmake)

string (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr" STANDARD_PREFIX)
if (NOT STANDARD_PREFIX)
	string (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr/local" STANDARD_PREFIX)
endif ()

if (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)
	add_definitions (-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif ()

set (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/cmake;/usr/local/share/apps/cmake/modules;/usr/share/apps/cmake/modules;${CMAKE_MODULE_PATH}")

enable_testing ()

set (LC_SOVERSION 0.6.75)

function (add_util_library name)
	cmake_parse_arguments (ARG "" "" "SOURCES;USES;DEPENDS" ${ARGN})
	set (lib_name "leechcraft-${name}")

	add_library (${lib_name} SHARED ${ARG_SOURCES})
	set_target_properties (${lib_name} PROPERTIES
		OUTPUT_NAME "${lib_name}${LC_LIBSUFFIX}"
		SOVERSION "${LC_SOVERSION}"
	)
	install (TARGETS ${lib_name} DESTINATION "${LIBDIR}")

	if (ARG_DEPENDS)
		target_link_libraries (${lib_name} ${ARG_DEPENDS})
	endif ()

	if (ARG_USES)
		FindQtLibs (${lib_name} ${ARG_USES})
	else ()
		FindQtLibs (${lib_name} Core)
	endif ()
endfunction ()

add_subdirectory (util)
add_subdirectory (xmlsettingsdialog)

set (LEECHCRAFT_LIBRARIES
		${LEECHCRAFT_UTIL_LIBRARIES}
		leechcraft-xsd${LC_LIBSUFFIX})

# Various options
option (STRICT_LICENSING "Makes LeechCraft more prepared, for example, to lacking artwork due to licensing restrictions" OFF)

if (STRICT_LICENSING)
	add_definitions (-DSTRICT_LICENSING)
endif ()

option (WITH_PLUGINS "Build a somewhat standard set of plugins as well" ON)
set (WITH_PLUGINS_WINDOWS OFF)
set (WITH_PLUGINS_APPLE OFF)
set (WITH_PLUGINS_UNIX OFF)
set (WITH_PLUGINS_UNIX_FREE OFF)
if (WITH_PLUGINS)
	if (WIN32)
		set (WITH_PLUGINS_WINDOWS ON)
	endif ()
	if (APPLE)
		set (WITH_PLUGINS_APPLE ON)
	endif ()
	if (UNIX)
		set (WITH_PLUGINS_UNIX ON)
	endif ()
	if (UNIX AND NOT APPLE)
		set (WITH_PLUGINS_UNIX_FREE ON)
	endif ()
endif ()

# Basic & traditional set of options
SUBPLUGIN (ADVANCEDNOTIFICATIONS "Build Advanced Notifications module for more customizable notifications" ${WITH_PLUGINS})
SUBPLUGIN (AGGREGATOR "Build Aggregator, the RSS/Atom feed reader" ${WITH_PLUGINS})
SUBPLUGIN (ANHERO "Build AnHero, the crash handler" ${WITH_PLUGINS_UNIX})
SUBPLUGIN (AUSCRIE "Build Auscrie, the auto screenshooter" ${WITH_PLUGINS})
SUBPLUGIN (AZOTH "Build Azoth, the IM client" ${WITH_PLUGINS})
SUBPLUGIN (BITTORRENT "Build BitTorrent, the BitTorrent client" ${WITH_PLUGINS})
SUBPLUGIN (DBUSMANAGER "Build DBusManager, the D-Bus support for the LeechCraft" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (DEADLYRICS "Build DeadLyrics, the client for the lyricwiki.org site" ${WITH_PLUGINS})
SUBPLUGIN (GLANCE "Quick overview of tabs" ${WITH_PLUGINS})
SUBPLUGIN (GMAILNOTIFIER "Build GMail Notifier which notifies about new mail in your GMail inbox" ${WITH_PLUGINS})
SUBPLUGIN (HISTORYHOLDER "Build HistoryHolder, the keeper of the history" ${WITH_PLUGINS})
SUBPLUGIN (CSTP "Build CSTP, the HTTP implementation" ${WITH_PLUGINS})
SUBPLUGIN (HTTHARE "Build HttThare for sharing local documents via HTTP" ${WITH_PLUGINS})
SUBPLUGIN (KINOTIFY "Build Kinotify, fancy kinetic notifications" ${WITH_PLUGINS})
SUBPLUGIN (KNOWHOW "Build KnowHow for displaying tips of the day" ${WITH_PLUGINS})
SUBPLUGIN (LMP "Build LMP, the LeechCraft Music Player" ${WITH_PLUGINS})
SUBPLUGIN (NETWORKMONITOR "Build NetworkMonitor, the monitor for HTTP requests" ${WITH_PLUGINS})
SUBPLUGIN (NEWLIFE "Build NewLife, the settings importer" ${WITH_PLUGINS})
SUBPLUGIN (PINTAB "Build Pintab for pinning important tabs" ${WITH_PLUGINS})
SUBPLUGIN (POGOOGLUE "Build Pogooglue, Google search client" ${WITH_PLUGINS})
SUBPLUGIN (POSHUKU "Build Poshuku, the WWW Browser" ${WITH_PLUGINS})
SUBPLUGIN (SECMAN "Build SecMan, security & personal data manager" ${WITH_PLUGINS})
SUBPLUGIN (SEEKTHRU "Build SeekThru, the client for the OpenSearch-aware web sites" ${WITH_PLUGINS})
SUBPLUGIN (SUMMARY "Build Summary, the plugin that shows a handy summary of the stuff going on" ${WITH_PLUGINS})
SUBPLUGIN (SYNCER "Build Syncer, the synchronization plugin for LeechCraft" OFF)
SUBPLUGIN (TABSESSMANAGER "Build Tab Session Manager" ${WITH_PLUGINS})
SUBPLUGIN (TABSLIST "Quick overview of tabs" ${WITH_PLUGINS})

# Traditional, but dependent on 3dparty
SUBPLUGIN (POPISHU "Build Popishu, a simple QScintilla-based text editor" OFF)
SUBPLUGIN (QROSP "Build Qrosp, the scripting support plugin" OFF)

# New
SUBPLUGIN (BLASQ "Build Blasq, client for cloud image storages like Picasa or Flickr" ${WITH_PLUGINS})
SUBPLUGIN (BLOGIQUE "Build Blogique, LeechCraft blogging client" ${WITH_PLUGINS})
SUBPLUGIN (CERTMGR "Build CertMgr for managing SSL certificates" ${WITH_PLUGINS})
SUBPLUGIN (CPULOAD "Build CpuLoad for, well, monitoring the CPU load" ${WITH_PLUGINS})
SUBPLUGIN (DEVMON "Build Devmon, the devices monitor plugin" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (DOLOZHEE "Build Dolozhee, the feature request and bug reporter" ${WITH_PLUGINS})
SUBPLUGIN (ELEEMINATOR "Build Leeminal, embedded LeechCraft terminal emulator" ${WITH_PLUGINS_UNIX})
SUBPLUGIN (FENET "Build Fenet, WM control plugin" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (FONTIAC "Build Fontiac for configurable font substitutions" ${WITH_PLUGINS})
SUBPLUGIN (GACTS "Build Global Actions provider plugin" ${WITH_PLUGINS})
SUBPLUGIN (HOTSENSORS "Build HotSensors, temperature sensors quark" ${WITH_PLUGINS_UNIX})
SUBPLUGIN (HOTSTREAMS "Build HotStreams, radio streams provider module" ${WITH_PLUGINS})
SUBPLUGIN (IMGASTE "Build Imgaste, simple image paster plugin" ${WITH_PLUGINS})
SUBPLUGIN (INTERMUTKO "Build Intermutko for configuring HTTP Accept-Language header" ${WITH_PLUGINS})
SUBPLUGIN (KBSWITCH "Build KBSwitch, a keyboard plugin" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (KRIGSTASK "Build Krigstask, applications switcher" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (LACKMAN "Build LackMan, the package manager for LeechCraft" ${WITH_PLUGINS})
SUBPLUGIN (LADS "Build Lads, Ubuntu Unity integration layer" OFF)
SUBPLUGIN (LASTFMSCROBBLE "Build Last.fm client plugin" ${WITH_PLUGINS})
SUBPLUGIN (LAUGHTY "Build Desktop Notifications server" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (LEMON "Build LeechCraft Network Monitor" ${WITH_PLUGINS})
SUBPLUGIN (LHTR "Build LeechCraft HTML Text editoR" ${WITH_PLUGINS})
SUBPLUGIN (LIZNOO "Build Liznoo, our nice power manager" ${WITH_PLUGINS})
SUBPLUGIN (MELLONETRAY "Build Mellonetray for providing tray area for third-party apps" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (MONOCLE "Build Monocle, modular document viewer for LeechCraft" ${WITH_PLUGINS})
SUBPLUGIN (MUSICZOMBIE "Build MusicZombie, MusicBrainz.org client" ${WITH_PLUGINS})
SUBPLUGIN (NACHEKU "Build Nacheku, clipboard/download dir watcher" ${WITH_PLUGINS})
SUBPLUGIN (NAMAUTH "Build NamAuth, providing standard HTTP authentication facilities" ${WITH_PLUGINS})
SUBPLUGIN (NETSTOREMANAGER "Build NetStoreManager for managing network data storages like Yandex.Disk" ${WITH_PLUGINS})
SUBPLUGIN (OTLOZHU "Build Otlozhu, a GTD-inspired ToDo manager" ${WITH_PLUGINS})
SUBPLUGIN (OORONEE "Build Ooronee, a quark for handling text and images dropped on it" ${WITH_PLUGINS})
SUBPLUGIN (POLEEMERY "Build Poleemery, the personal finances manager" ${WITH_PLUGINS})
SUBPLUGIN (ROSENTHAL "Build Rosenthal, the Hunspell-based spell checker service plugin" ${WITH_PLUGINS})
SUBPLUGIN (SB2 "Build SB2, next-gen fluid sidebar" ${WITH_PLUGINS})
SUBPLUGIN (SCROBLIBRE "Build ScrobLibre, multiaccount Scrobbler API 1.2 scrobbler" ${WITH_PLUGINS})
SUBPLUGIN (SYSNOTIFY "Build Sysnotify for using D-Bus notification API" ${WITH_PLUGINS_UNIX_FREE})
SUBPLUGIN (TEXTOGROOSE "Build Text-o-Groose, script-based lyrics fetcher" ${WITH_PLUGINS})
SUBPLUGIN (TOUCHSTREAMS "Build TouchStreams, VK.com music streaming plugin" ${WITH_PLUGINS})
SUBPLUGIN (TPI "Build Task Progress Indicator quark plugin" ${WITH_PLUGINS})
SUBPLUGIN (VROOBY "Build Vrooby, removable storage devices manager" ${WITH_PLUGINS})
SUBPLUGIN (XPROXY "Build XProxy, the advanced proxy manager" ${WITH_PLUGINS})
SUBPLUGIN (XTAZY "Build Xtazy, plugin for keeping track of a currently playing song" ${WITH_PLUGINS})
SUBPLUGIN (ZALIL "Build Zalil for uploading files to accountless filebin services" ${WITH_PLUGINS})

SUBPLUGIN (CHOROID "Build Choroid, image viewer" OFF)
SUBPLUGIN (SNAILS "Build Snails, LeechCraft mail client" OFF)

# Dumb alternatives
SUBPLUGIN (DUMBEEP "Build Dumbeep, a dumb sound notifier for those fearing true LMP power" OFF)

SUBPLUGIN (Y7 "Build Y7, Windows 7 integration plugin" ${WITH_PLUGINS_WINDOWS})
SUBPLUGIN (LAUNCHY "Build Launchy, third-party application launcher module" ${WITH_PLUGINS_UNIX})
SUBPLUGIN (PIERRE "Build Pierre, Mac OS X integration layer" ${WITH_PLUGINS_APPLE})

# Defunct
SUBPLUGIN (OTZERKALU "Build Otzerkalu, which allows one to download web sites recursively" OFF)

if (NOT DEFINED LEECHCRAFT_VERSION)
	find_program (GIT_CMD git)
	execute_process (COMMAND "${GIT_CMD}" "--git-dir=${CMAKE_CURRENT_SOURCE_DIR}/../.git" describe
		OUTPUT_VARIABLE LEECHCRAFT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	if (NOT DEFINED LEECHCRAFT_VERSION)
		message (ERROR "Please define LeechCraft version")
	endif ()
	message (STATUS "Detected LeechCraft version: ${LEECHCRAFT_VERSION}")
endif ()

configure_file (config.h.in config.h)
configure_file (FindLeechCraft.cmake.in FindLeechCraft-qt5.cmake @ONLY)
install (FILES
	${CMAKE_CURRENT_BINARY_DIR}/FindLeechCraft-qt5.cmake
	CommonLCSetup.cmake
	DESTINATION ${LC_SHARE_DEST}/cmake/)

if (APPLE AND NOT USE_UNIX_LAYOUT)
	configure_file (Info.plist.in Info.plist)
	install (FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist DESTINATION ${LC_BINDIR}/../)
endif ()

add_subdirectory (core)

install (DIRECTORY util/ DESTINATION include/leechcraft/util/
	FILES_MATCHING PATTERN "*.h")
install (DIRECTORY interfaces/ DESTINATION include/leechcraft/interfaces/
	FILES_MATCHING PATTERN "*.h")
install (FILES xmlsettingsdialog/xmlsettingsdialog.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES xmlsettingsdialog/basesettingsmanager.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES xmlsettingsdialog/xsdconfig.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES xmlsettingsdialog/datasourceroles.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/leechcraft/)

install (DIRECTORY share/ DESTINATION ${LC_SHARE_DEST})

install (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor)
if (UNIX AND NOT APPLE)
	install (FILES InitLCPlugin.cmake DESTINATION share/cmake/Modules)
	install (FILES freedesktop/leechcraft-qt5.desktop DESTINATION share/applications)
	install (FILES scripts/leechcraft-add-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
	install (FILES scripts/leechcraft-handle-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)

	if (NOT SKIP_MAN_COMPRESS)
		add_custom_target (data_leechcraft_gz ALL
			"${CMAKE_CURRENT_SOURCE_DIR}/data/compress"
			DEPENDS data/leechcraft-qt5.1
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
			)
		install (DIRECTORY data/ DESTINATION share/man/man1 FILES_MATCHING PATTERN "*.gz")
	else ()
		install (DIRECTORY data/ DESTINATION share/man/man1 FILES_MATCHING PATTERN "*.1")
	endif ()
endif ()
