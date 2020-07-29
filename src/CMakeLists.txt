cmake_minimum_required (VERSION 3.10)

project (leechcraft)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (CMAKE_CXX_EXTENSIONS OFF)

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
if (UNIX OR (WIN32 AND MINGW))
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall -Wextra")
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_DEPRECATED_WARNINGS -DQT_DISABLE_DEPRECATED_BEFORE='QT_VERSION_CHECK(5,8,0)'")

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
	set (LC_QML_DEST "share/leechcraft/qml")
	set (LC_QML5_DEST "share/leechcraft/qml5")
	set (LC_SCRIPTS_DEST "share/leechcraft/scripts")
	set (LC_SHARE_DEST "share/leechcraft")
	set (LC_INSTALLEDMANIFEST_DEST "share/leechcraft/installed")
	set (LC_GLOBAL_SETTINGS_DEST "share/leechcraft/rc")
	set (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
elseif (WIN32)
	if (NOT LIBDIR)
		set (LIBDIR "${CMAKE_INSTALL_PREFIX}")
	endif ()

	set (LC_BINDIR "${CMAKE_INSTALL_PREFIX}")
	set (LC_PLUGINS_DEST "plugins/bin")
	set (LC_TRANSLATIONS_DEST "translations")
	set (LC_SETTINGS_DEST "settings")
	set (LC_QML_DEST "share/qml")
	set (LC_QML5_DEST "share/qml5")
	set (LC_SCRIPTS_DEST "share/scripts")
	set (LC_SHARE_DEST "share")
	set (LC_INSTALLEDMANIFEST_DEST "share/installed")
	set (LC_GLOBAL_SETTINGS_DEST "settings")
	set (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
else ()
	set (LIBDIR "leechcraft.app/Contents/Frameworks")
	set (LC_BINDIR "leechcraft.app/Contents/MacOs")
	set (LC_PLUGINS_DEST "leechcraft.app/Contents/PlugIns")
	set (LC_TRANSLATIONS_DEST "leechcraft.app/Contents/Resources/translations")
	set (LC_SETTINGS_DEST "leechcraft.app/Contents/Resources/settings")
	set (LC_QML_DEST "leechcraft.app/Contents/Resources/share/qml")
	set (LC_QML5_DEST "leechcraft.app/Contents/Resources/share/qml5")
	set (LC_SCRIPTS_DEST "leechcraft.app/Contents/Resources/share/scripts")
	set (LC_SHARE_DEST "leechcraft.app/Contents/Resources/share")
	set (LC_INSTALLEDMANIFEST_DEST "leechcraft.app/Contents/Resources/share/installed")
	set (LC_GLOBAL_SETTINGS_DEST "leechcraft.app/Contents/Resources/share/rc")
	set (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
endif ()

find_package (Boost REQUIRED COMPONENTS date_time filesystem program_options system thread)

set (CMAKE_INCLUDE_CURRENT_DIR ON)
find_package (Qt5Widgets)
set (LC_LIBSUFFIX "-qt5")

macro (QtWrapUi outfiles)
	set (UIS_H)
	QT5_WRAP_UI (UIS_H ${ARGN})
	set (${outfiles} ${${outfiles}} ${UIS_H})
endmacro ()

macro (QtAddResources outfiles)
	set (RCCS)
	QT5_ADD_RESOURCES (RCCS ${ARGN} OPTIONS -compress-algo best -threshold 0)
	set (${outfiles} ${${outfiles}} ${RCCS})
endmacro ()

macro (FindQtLibs Target)
	cmake_policy (SET CMP0043 NEW)
	set (CMAKE_INCLUDE_CURRENT_DIR ON)

	set (_TARGET_QT_COMPONENTS "")
	foreach (V ${ARGN})
		if (NOT (${V} MATCHES ".*Private$"))
			list (APPEND _TARGET_QT_COMPONENTS ${V})
		endif ()
	endforeach ()
	find_package (Qt5 COMPONENTS ${_TARGET_QT_COMPONENTS})

	set (_TARGET_LINK_QT5_LIBS "")
	foreach (V ${ARGN})
		list (APPEND _TARGET_LINK_QT5_LIBS "Qt5::${V}")
	endforeach ()
	target_link_libraries (${Target} ${_TARGET_LINK_QT5_LIBS})
endmacro ()

string (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr" STANDARD_PREFIX)
if (NOT STANDARD_PREFIX)
	string (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr/local" STANDARD_PREFIX)
endif ()

if (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)
	add_definitions (-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif ()

set (CMAKE_AUTOMOC TRUE)

set (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/cmake;/usr/local/share/apps/cmake/modules;/usr/share/apps/cmake/modules;${CMAKE_MODULE_PATH}")

enable_testing ()

set (LC_SOVERSION 0.6.75)

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

find_package(Qt5LinguistTools REQUIRED)

function (CreateTrs CompiledTranVar)
	file (GLOB TS_SOURCES "*.ts")
	if (TS_SOURCES)
		list (TRANSFORM TS_SOURCES REPLACE "(.*)\\.ts" "\\1.qm" OUTPUT_VARIABLE QM_RESULTS)
		add_custom_command (OUTPUT ${QM_RESULTS}
			COMMAND Qt5::lrelease ${TS_SOURCES}
			DEPENDS ${TS_SOURCES}
			)
		install (FILES ${QM_RESULTS} DESTINATION ${LC_TRANSLATIONS_DEST})
	else ()
		set (QM_RESULTS)
	endif ()
	set (${CompiledTranVar} ${QM_RESULTS})
endfunction ()

function(LC_DEFINE_PLUGIN)
	set (options INSTALL_SHARE INSTALL_DESKTOP)
	set (one_value_args RESOURCES)
	set (multi_value_args SRCS SETTINGS QT_COMPONENTS LINK_LIBRARIES)
	cmake_parse_arguments (P "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

	include_directories (
		${CMAKE_CURRENT_BINARY_DIR}
		${LEECHCRAFT_INCLUDE_DIR}
	)

	CreateTrs (QM_RESULTS)

	set (FULL_NAME ${PROJECT_NAME})

	add_library (${FULL_NAME} SHARED
		${QM_RESULTS}
		${P_SRCS}
		${P_RESOURCES}
		)
	set_target_properties (${FULL_NAME} PROPERTIES AUTOUIC TRUE AUTORCC TRUE)
	target_link_libraries (${FULL_NAME} ${LEECHCRAFT_LIBRARIES} ${P_LINK_LIBRARIES})

	install (TARGETS ${FULL_NAME} DESTINATION ${LC_PLUGINS_DEST})

	if (P_INSTALL_SHARE)
		install (DIRECTORY share/ DESTINATION ${LC_SHARE_DEST})
	endif ()

	if (P_SETTINGS)
		install (FILES ${P_SETTINGS} DESTINATION ${LC_SETTINGS_DEST})
	endif ()

	if (P_INSTALL_DESKTOP AND UNIX AND NOT APPLE)
		install (DIRECTORY freedesktop/ DESTINATION share/applications)
	endif ()

	FindQtLibs (${FULL_NAME} ${P_QT_COMPONENTS})
endfunction()

function (SUBPLUGIN suffix descr)
	string (TOUPPER ${PROJECT_NAME} PROJECT_NAME_UPPER)
	string (SUBSTRING ${PROJECT_NAME_UPPER} 10 -1 root_name)

	set (defVal "ON")
	if (NOT ${ARGN})
		set (defVal "OFF")
	endif ()
	string (TOLOWER ${suffix} suffixL)
	option (ENABLE${root_name}_${suffix} "${descr}" ${defVal})
	if (ENABLE${root_name}_${suffix})
		include_directories (BEFORE ${CMAKE_CURRENT_SOURCE_DIR})
		add_subdirectory (plugins/${suffixL})
	endif ()
endfunction ()

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
SUBPLUGIN (SYNCER "Build Syncer, the synchronization plugin for LeechCraft" FALSE)
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
SUBPLUGIN (WKPLUGINS "Build WKPlugins for WebKit spellcheck and notifications" OFF)
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
		if (WIN32)
			execute_process(COMMAND "date" "/T" OUTPUT_VARIABLE
				${LEECHCRAFT_VERSION})
			string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
				${LEECHCRAFT_VERSION} ${${LEECHCRAFT_VERSION}})
		elseif(UNIX)
			execute_process(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE
				${LEECHCRAFT_VERSION})
			string(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
				${LEECHCRAFT_VERSION} ${${LEECHCRAFT_VERSION}})
		else ()
			message(SEND_ERROR "date not implemented")
			set(${RESULT} 000000)
		endif ()
	endif ()
	message (STATUS "Detected LeechCraft version: ${LEECHCRAFT_VERSION}")
endif ()

configure_file (config.h.in config.h)
configure_file (FindLeechCraft.cmake.in FindLeechCraft-qt5.cmake @ONLY)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/FindLeechCraft-qt5.cmake DESTINATION ${LC_SHARE_DEST}/cmake/)

if (APPLE)
	configure_file (Info.plist.in Info.plist)
	if (NOT USE_UNIX_LAYOUT)
		install (FILES ${CMAKE_CURRENT_BINARY_DIR}/Info.plist DESTINATION ${LC_BINDIR}/../)
	endif ()
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

install (DIRECTORY share/leechcraft/ DESTINATION ${LC_SHARE_DEST})
install (DIRECTORY share/qml5/common DESTINATION ${LC_QML5_DEST}/org/LC)

install (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor)
if (UNIX AND NOT APPLE)
	install (FILES InitLCPlugin.cmake DESTINATION share/cmake/Modules)
	install (DIRECTORY cmake/ DESTINATION share/leechcraft/cmake)
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
