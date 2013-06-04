cmake_minimum_required(VERSION 2.6)
if (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
endif (COMMAND cmake_policy)
project (leechcraft)
#if (NOT DISABLE_RPATH_TRICKS)
#	set (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#	set (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#	set (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
#endif (NOT DISABLE_RPATH_TRICKS)
if (MSVC)
	set (CMAKE_CXX_FLAGS "/EHa /W3")
	set (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD /D NDEBUG")
	set (CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /GT /MDd")

	set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
	include (LibHelperWin32Macros)
endif (MSVC)
if (WIN32)
	add_definitions (-DWIN32_LEAN_AND_MEAN) # Don't include not needed headers with a lot of useless macross definitions
	add_definitions (-D_WIN32_WINNT=0x0600) # Use Windows Vista definitions. Needed by Liznoo and KInotify
	add_definitions (-DWINVER=0x0600)
	add_definitions (-DNTDDI_VERSION=0x0600)
endif (WIN32)
if (UNIX OR (WIN32 AND MINGW))
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=c++0x")

	if (NOT APPLE)
		set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
		set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
	endif (NOT APPLE)

	if (EXTRA_WARNINGS)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	endif (EXTRA_WARNINGS)

	if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND ENABLE_CLANG_QUIRKS)
		message (STATUS "Enable Clang-specific quirks for Qt...")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_RVALUE_REFS -DQ_COMPILER_DECLTYPE")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_VARIADIC_TEMPLATES -DQ_COMPILER_AUTO_TYPE -DQ_COMPILER_EXTERN_TEMPLATES -DQ_COMPILER_DEFAULT_DELETE_MEMBERS -DQ_COMPILER_CLASS_ENUM -DQ_COMPILER_INITIALIZER_LISTS")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_LAMBDA -DQ_COMPILER_UNICODE_STRINGS")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_CONSTEXPR")
	endif (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND ENABLE_CLANG_QUIRKS)
endif (UNIX OR (WIN32 AND MINGW))

option (USE_QT5 "Enable Qt5 build" OFF)

if (GPTOOLS_CPU AND GPTOOLS_MEM)
	set (GPTOOLS_CPU FALSE)
	set (GPTOOLS_MEM FALSE)
	set (GPTOOLS_CPU_AND_MEM TRUE)
endif (GPTOOLS_CPU AND GPTOOLS_MEM)
if (GPTOOLS_MEM)
	message (STATUS "Enabled memory profiling via Google perftools")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc")
endif (GPTOOLS_MEM)
if (GPTOOLS_CPU)
	message (STATUS "Enabled CPU profiling via Google perftools")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
endif (GPTOOLS_CPU)
if (GPTOOLS_CPU_AND_MEM)
	message (STATUS "Enabled both CPU and memory profiling via Google perftools")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc_and_profiler")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc_and_profiler")
endif (GPTOOLS_CPU_AND_MEM)

if (UNIX AND NOT APPLE)
	if (NOT LIBDIR)
		set (LIBDIR "lib${LIB_SUFFIX}")
	endif (NOT LIBDIR)

	set (LC_BINDIR "bin")
	set (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins")
	set (LC_TRANSLATIONS_DEST "share/leechcraft/translations")
	set (LC_SETTINGS_DEST "share/leechcraft/settings")
	set (LC_QML_DEST "share/leechcraft/qml")
	set (LC_SHARE_DEST "share/leechcraft")
	set (LC_INSTALLEDMANIFEST_DEST "share/leechcraft/installed")
	set (LC_GLOBAL_SETTINGS_DEST "share/leechcraft/rc")
	set (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
elseif (WIN32)
	if (NOT LIBDIR)
		set (LIBDIR "${CMAKE_INSTALL_PREFIX}")
	endif (NOT LIBDIR)

	set (LC_BINDIR "${CMAKE_INSTALL_PREFIX}")
	set (LC_PLUGINS_DEST "plugins/bin")
	set (LC_TRANSLATIONS_DEST "translations")
	set (LC_SETTINGS_DEST "settings")
	set (LC_QML_DEST "share/qml")
	set (LC_SHARE_DEST "share")
	set (LC_INSTALLEDMANIFEST_DEST "share/installed")
	set (LC_GLOBAL_SETTINGS_DEST "settings")
	set (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
else (UNIX AND NOT APPLE)
	set (LIBDIR "lib")
	set (LC_BINDIR "leechcraft.app/Contents/MacOs")
	set (LC_PLUGINS_DEST "leechcraft.app/Contents/PlugIns")
	set (LC_TRANSLATIONS_DEST "leechcraft.app/Contents/Resources/translations")
	set (LC_SETTINGS_DEST "leechcraft.app/Contents/Resources/settings")
	set (LC_QML_DEST "leechcraft.app/Contents/Resources/share/qml")
	set (LC_SHARE_DEST "leechcraft.app/Contents/Resources/share")
	set (LC_INSTALLEDMANIFEST_DEST "leechcraft.app/Contents/Resources/share/installed")
	set (LC_GLOBAL_SETTINGS_DEST "leechcraft.app/Contents/Resources/share/rc")
	set (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
endif (UNIX AND NOT APPLE)

set (Boost_ADDITIONAL_VERSIONS "1.47" "1.46" "1.46.1" "1.44" "1.44.0")
find_package (Boost REQUIRED COMPONENTS date_time filesystem program_options system thread)

if (USE_QT5)
	set (CMAKE_INCLUDE_CURRENT_DIR ON)
	find_package (Qt5Widgets)
else (USE_QT5)
	find_package (Qt4 REQUIRED)
endif (USE_QT5)

string (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr" STANDARD_PREFIX)
if (NOT STANDARD_PREFIX)
	string (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr/local" STANDARD_PREFIX)
endif ()

if (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)
	add_definitions (-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)

if (NOT CMAKE_VERSION VERSION_LESS 2.8.6)
	message (STATUS "CMake 2.8.6 or greater found, enabling automoc...")
	set (CMAKE_AUTOMOC TRUE)
	set (LC_NO_MOC TRUE)
endif (NOT CMAKE_VERSION VERSION_LESS 2.8.6)

set (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
set (LEECHCRAFT_LIBRARIES lcutil xmlsettingsdialog)

# Various options
option (STRICT_LICENSING "Makes LeechCraft more prepared, for example, to lacking artwork due to licensing restrictions" OFF)

if (STRICT_LICENSING)
	add_definitions (-DSTRICT_LICENSING)
endif (STRICT_LICENSING)

option (WITH_PLUGINS "Build a somewhat standard set of plugins as well" ON)

# Basic & traditional set of options
option (ENABLE_ADVANCEDNOTIFICATIONS "Build Advanced Notifications module for more customizable notifications" ${WITH_PLUGINS})
option (ENABLE_AGGREGATOR "Build Aggregator, the RSS/Atom feed reader" ${WITH_PLUGINS})
option (ENABLE_ANHERO "Build AnHero, the crash handler" ${WITH_PLUGINS})
option (ENABLE_AUSCRIE "Build Auscrie, the auto screenshooter" ${WITH_PLUGINS})
option (ENABLE_AZOTH "Build Azoth, the IM client" ${WITH_PLUGINS})
option (ENABLE_DBUSMANAGER "Build DBusManager, the D-Bus support for the LeechCraft" ${WITH_PLUGINS})
option (ENABLE_DEADLYRICS "Build DeadLyrics, the client for the lyricwiki.org site" ${WITH_PLUGINS})
option (ENABLE_GLANCE "Quick overview of tabs" ${WITH_PLUGINS})
option (ENABLE_GMAILNOTIFIER "Build GMail Notifier which notifies about new mail in your GMail inbox" ${WITH_PLUGINS})
option (ENABLE_HISTORYHOLDER "Build HistoryHolder, the keeper of the history" ${WITH_PLUGINS})
option (ENABLE_HTTP "Build CSTP, the HTTP implementation" ${WITH_PLUGINS})
option (ENABLE_KINOTIFY "Build Kinotify, fancy kinetic notifications" ${WITH_PLUGINS})
option (ENABLE_KNOWHOW "Build KnowHow for displaying tips of the day" ${WITH_PLUGINS})
option (ENABLE_LMP "Build LMP, the LeechCraft Music Player" ${WITH_PLUGINS})
option (ENABLE_NETWORKMONITOR "Build NetworkMonitor, the monitor for HTTP requests" ${WITH_PLUGINS})
option (ENABLE_NEWLIFE "Build NewLife, the settings importer" ${WITH_PLUGINS})
option (ENABLE_PINTAB "Build Pintab for pinning important tabs" ${WITH_PLUGINS})
option (ENABLE_POGOOGLUE "Build Pogooglue, Google search client" ${WITH_PLUGINS})
option (ENABLE_POSHUKU "Build Poshuku, the WWW Browser" ${WITH_PLUGINS})
option (ENABLE_SECMAN "Build SecMan, security & personal data manager" ${WITH_PLUGINS})
option (ENABLE_SEEKTHRU "Build SeekThru, the client for the OpenSearch-aware web sites" ${WITH_PLUGINS})
option (ENABLE_SUMMARY "Build Summary, the plugin that shows a handy summary of the stuff going on" ${WITH_PLUGINS})
option (ENABLE_SYNCER "Build Syncer, the synchronization plugin for LeechCraft" ${WITH_PLUGINS})
option (ENABLE_TABSESSMANAGER "Build Tab Session Manager" ${WITH_PLUGINS})
option (ENABLE_TABSLIST "Quick overview of tabs" ${WITH_PLUGINS})
option (ENABLE_TORRENT "Build BitTorrent, the BitTorrent client" ${WITH_PLUGINS})
option (ENABLE_VGRABBER "Build vGrabber, the vkontakte.ru music/video getter" ${WITH_PLUGINS})

# Traditional, but dependent on 3dparty
option (ENABLE_POPISHU "Build Popishu, a simple QScintilla-based text editor" OFF)
option (ENABLE_QROSP "Build Qrosp, the scripting support plugin" OFF)

# New
option (ENABLE_BLOGIQUE "Build Blogique, LeechCraft blogging client" ${WITH_PLUGINS})
option (ENABLE_DOLOZHEE "Build Dolozhee, the feature request and bug reporter" ${WITH_PLUGINS})
option (ENABLE_GACTS "Build Global Actions provider plugin" ${WITH_PLUGINS})
option (ENABLE_HOTSENSORS "Build HotSensors, temperature sensors quark" ${WITH_PLUGINS})
option (ENABLE_HOTSTREAMS "Build HotStreams, radio streams provider module" ${WITH_PLUGINS})
option (ENABLE_KBSWITCH "Build KBSwitch, a keyboard plugin" ${WITH_PLUGINS})
option (ENABLE_KRIGSTASK "Build Krigstask, applications switcher" ${WITH_PLUGINS})
option (ENABLE_LACKMAN "Build LackMan, the package manager for LeechCraft" ${WITH_PLUGINS})
option (ENABLE_LADS "Build Lads, Ubuntu Unity integration layer" ${WITH_PLUGINS})
option (ENABLE_LASTFMSCROBBLE "Build Last.fm scrobbler" ${WITH_PLUGINS})
option (ENABLE_LAUGHTY "Build Desktop Notifications server" ${WITH_PLUGINS})
option (ENABLE_LEMON "Build LeechCraft Network Monitor" ${WITH_PLUGINS})
option (ENABLE_LHTR "Build LeechCraft HTML Text editoR" ${WITH_PLUGINS})
option (ENABLE_LIZNOO "Build Liznoo, our nice power manager" ${WITH_PLUGINS})
option (ENABLE_MELLONETRAY "Build Mellonetray for providing tray area for third-party apps" ${WITH_PLUGINS})
option (ENABLE_MONOCLE "Build Monocle, modular document viewer for LeechCraft" ${WITH_PLUGINS})
option (ENABLE_MUSICZOMBIE "Build MusicZombie, MusicBrainz.org client" ${WITH_PLUGINS})
option (ENABLE_NACHEKU "Build Nacheku, clipboard/download dir watcher" ${WITH_PLUGINS})
option (ENABLE_NETSTOREMANAGER "Build NetStoreManager for managing network data storages like Yandex.Disk" ${WITH_PLUGINS})
option (ENABLE_OTLOZHU "Build Otlozhu, a GTD-inspired ToDo manager" ${WITH_PLUGINS})
option (ENABLE_POLEEMERY "Build Poleemery, the personal finances manager" ${WITH_PLUGINS})
option (ENABLE_SB2 "Build SB2, next-gen fluid sidebar" ${WITH_PLUGINS})
option (ENABLE_SHAITAN "Build Shaitan, Embedded terminal emulator" ${WITH_PLUGINS})
option (ENABLE_SYSNOTIFY "Build Sysnotify for using D-Bus notification API" ${WITH_PLUGINS})
option (ENABLE_TOUCHSTREAMS "Build TouchStreams, VK.com music streaming plugin" ${WITH_PLUGINS})
option (ENABLE_TPI "Build Task Progress Indicator quark plugin" ${WITH_PLUGINS})
option (ENABLE_VROOBY "Build Vrooby, removable storage devices manager" ${WITH_PLUGINS})
option (ENABLE_XPROXY "Build XProxy, the advanced proxy manager" ${WITH_PLUGINS})
option (ENABLE_XTAZY "Build Xtazy, plugin for keeping track of a currently playing song" ${WITH_PLUGINS})

option (ENABLE_BLACKDASH "Build BlackDash, the dashboard plugin for LeechCraft" OFF)
option (ENABLE_CHOROID "Build Choroid, image viewer" OFF)
option (ENABLE_SNAILS "Build Snails, LeechCraft mail client" OFF)
option (ENABLE_TWIFEE "Build Twifee, the Twitter client" OFF)
option (ENABLE_CEMENTO "Build Cemento, LeechCraft common persistent storage" OFF)
option (ENABLE_WOODPECKER "Build Woodpecker, twitter client" ${WITH_PLUGINS})

# Dumb alternatives
option (ENABLE_DUMBEEP "Build Dumbeep, a dumb sound notifier for those fearing true LMP power" OFF)

if (WIN32)
	option (ENABLE_Y7 "Build Y7, Windows 7 integration plugin" ${WITH_PLUGINS})
endif (WIN32)

if (UNIX)
	option (ENABLE_LAUNCHY "Build Launchy, third-party application launcher module" ${WITH_PLUGINS})
endif (UNIX)

if (APPLE)
	option (ENABLE_PIERRE "Build Pierre, Mac OS X integration layer" ${WITH_PLUGINS})
endif (APPLE)

# Old ones
option (ENABLE_SHELLOPEN "Build ShellOpen, the plugin supporting handling entities via external applications" OFF)

# Defunct
option (ENABLE_JUFFED "Build JuffEd, the advanced text editor" OFF)
option (ENABLE_OTZERKALU "Build Otzerkalu, which allows one to download web sites recursively" OFF)
option (ENABLE_LAURE "Build Laure, the new generation audio/video player" OFF)

enable_testing ()

function (CreateTrs PlugName Langs CompiledTranVar)
	string (LENGTH "${PlugName}" _pnameLength)
	set (_basename "leechcraft")
	if (_pnameLength)
		set (_basename "${_basename}_${PlugName}")
	endif (_pnameLength)
	foreach (_LANG ${Langs})
		set (_tran ${_tran} ${_basename}_${_LANG}.ts)
		set (_ctran ${_ctran} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}_${_LANG}.qm)
	endforeach (_LANG)
	set (${CompiledTranVar} ${_ctran} PARENT_SCOPE)

	add_custom_command (OUTPUT ${_ctran}
		COMMAND "${QT_LRELEASE_EXECUTABLE}" ${_tran}
		DEPENDS ${_tran}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	install (FILES ${_ctran} DESTINATION ${LC_TRANSLATIONS_DEST})
	set (${CompiledTranVar} ${_ctran} PARENT_SCOPE)
endfunction (CreateTrs)

set (GENCPP_XML_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../tools/scripts/translationstuff/gencpp.sh")

function (CreateTrsUpTarget PlugName Langs Sources Forms Xml)
	string (LENGTH "${PlugName}" _pnameLength)
	string (LENGTH "${Xml}" _xmlLength)
	set (_basename "leechcraft")
	if (_pnameLength)
		set (_basename "${_basename}_${PlugName}")
	endif (_pnameLength)
	if (_xmlLength)
		foreach (_LANG ${Langs})
			add_custom_target (${_basename}_ts_${_LANG}
					${GENCPP_XML_PATH} ${Xml}
					COMMAND ${QT_LUPDATE_EXECUTABLE} "-noobsolete" ${Sources} ${Forms} "dummy.cpp" "-ts" "${_basename}_${_LANG}.ts"
					COMMAND "rm" "-f" "dummy.cpp"
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
		endforeach (_LANG)
	else (_xmlLength)
		foreach (_LANG ${Langs})
			add_custom_target (${_basename}_ts_${_LANG}
					${QT_LUPDATE_EXECUTABLE} "-noobsolete" ${Sources} ${Forms} "-ts" "${_basename}_${_LANG}.ts"
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
		endforeach (_LANG)
	endif (_xmlLength)
endfunction (CreateTrsUpTarget)

if (ENABLE_SUMMARY)
	add_subdirectory (plugins/summary)
endif (ENABLE_SUMMARY)

if (ENABLE_TORRENT)
	add_subdirectory (plugins/bittorrent)
endif (ENABLE_TORRENT)

if (ENABLE_AGGREGATOR)
	add_subdirectory (plugins/aggregator)
endif (ENABLE_AGGREGATOR)

if (ENABLE_CHOROID)
	add_subdirectory (plugins/choroid)
endif (ENABLE_CHOROID)

if (ENABLE_POSHUKU)
	add_subdirectory (plugins/poshuku)
endif (ENABLE_POSHUKU)

if (ENABLE_HTTP)
	add_subdirectory (plugins/cstp)
endif (ENABLE_HTTP)

if (ENABLE_LMP)
	add_subdirectory (plugins/lmp)
endif (ENABLE_LMP)

if (ENABLE_DBUSMANAGER)
	add_subdirectory (plugins/dbusmanager)
endif (ENABLE_DBUSMANAGER)

if (ENABLE_DEADLYRICS)
	add_subdirectory (plugins/deadlyrics)
endif (ENABLE_DEADLYRICS)

if (ENABLE_SEEKTHRU)
	add_subdirectory (plugins/seekthru)
endif (ENABLE_SEEKTHRU)

if (ENABLE_NETWORKMONITOR)
	add_subdirectory (plugins/networkmonitor)
endif (ENABLE_NETWORKMONITOR)

if (ENABLE_HISTORYHOLDER)
	add_subdirectory (plugins/historyholder)
endif (ENABLE_HISTORYHOLDER)

if (ENABLE_NEWLIFE)
	add_subdirectory (plugins/newlife)
endif (ENABLE_NEWLIFE)

if (ENABLE_ANHERO)
	add_subdirectory (plugins/anhero)
endif (ENABLE_ANHERO)

if (ENABLE_VGRABBER)
	add_subdirectory (plugins/vgrabber)
endif (ENABLE_VGRABBER)

if (ENABLE_KINOTIFY)
	add_subdirectory (plugins/kinotify)
endif (ENABLE_KINOTIFY)

if (ENABLE_AUSCRIE)
	add_subdirectory (plugins/auscrie)
endif (ENABLE_AUSCRIE)

if (ENABLE_AZOTH)
	add_subdirectory (plugins/azoth)
endif (ENABLE_AZOTH)

if (ENABLE_SECMAN)
	add_subdirectory (plugins/secman)
endif (ENABLE_SECMAN)

if (ENABLE_QROSP)
	add_subdirectory (plugins/qrosp)
endif (ENABLE_QROSP)

if (ENABLE_POPISHU)
	add_subdirectory (plugins/popishu)
endif (ENABLE_POPISHU)

if (ENABLE_HOTSENSORS)
	add_subdirectory (plugins/hotsensors)
endif (ENABLE_HOTSENSORS)

if (ENABLE_HOTSTREAMS)
	add_subdirectory (plugins/hotstreams)
endif (ENABLE_HOTSTREAMS)

if (ENABLE_KRIGSTASK)
	add_subdirectory (plugins/krigstask)
endif (ENABLE_KRIGSTASK)

if (ENABLE_LACKMAN)
	add_subdirectory (plugins/lackman)
endif (ENABLE_LACKMAN)

if (ENABLE_LAUNCHY)
	add_subdirectory (plugins/launchy)
endif (ENABLE_LAUNCHY)

if (ENABLE_SYNCER)
	add_subdirectory (plugins/syncer)
endif (ENABLE_SYNCER)

if (ENABLE_SHELLOPEN)
	add_subdirectory (plugins/shellopen)
endif (ENABLE_SHELLOPEN)

if (ENABLE_LAURE)
	add_subdirectory (plugins/laure)
endif (ENABLE_LAURE)

if (ENABLE_JUFFED)
	add_subdirectory (plugins/juffed)
endif (ENABLE_JUFFED)

if (ENABLE_GMAILNOTIFIER)
	add_subdirectory (plugins/gmailnotifier)
endif (ENABLE_GMAILNOTIFIER)

if (ENABLE_OTZERKALU)
	add_subdirectory (plugins/otzerkalu)
endif (ENABLE_OTZERKALU)

if (ENABLE_ADVANCEDNOTIFICATIONS)
	add_subdirectory (plugins/advancednotifications)
endif (ENABLE_ADVANCEDNOTIFICATIONS)

if (ENABLE_GLANCE)
	add_subdirectory (plugins/glance)
endif (ENABLE_GLANCE)

if (ENABLE_TABSLIST)
	add_subdirectory (plugins/tabslist)
endif (ENABLE_TABSLIST)

if (ENABLE_BLACKDASH)
	add_subdirectory (plugins/blackdash)
endif (ENABLE_BLACKDASH)

if (ENABLE_KNOWHOW)
	add_subdirectory (plugins/knowhow)
endif (ENABLE_KNOWHOW)

if (ENABLE_POLEEMERY)
	add_subdirectory (plugins/poleemery)
endif (ENABLE_POLEEMERY)

if (ENABLE_SB2)
	add_subdirectory (plugins/sb2)
endif (ENABLE_SB2)

if (ENABLE_SNAILS)
	add_subdirectory (plugins/snails)
endif (ENABLE_SNAILS)

if (ENABLE_SYSNOTIFY)
	add_subdirectory (plugins/sysnotify)
endif (ENABLE_SYSNOTIFY)

if (ENABLE_TOUCHSTREAMS)
	add_subdirectory (plugins/touchstreams)
endif (ENABLE_TOUCHSTREAMS)

if (ENABLE_TPI)
	add_subdirectory (plugins/tpi)
endif (ENABLE_TPI)

if (ENABLE_TWIFEE)
	add_subdirectory (plugins/twifee)
endif (ENABLE_TWIFEE)

if (ENABLE_LIZNOO)
	add_subdirectory (plugins/liznoo)
endif (ENABLE_LIZNOO)

if (ENABLE_MUSICZOMBIE)
	add_subdirectory (plugins/musiczombie)
endif (ENABLE_MUSICZOMBIE)

if (ENABLE_NETSTOREMANAGER)
	add_subdirectory (plugins/netstoremanager)
endif (ENABLE_NETSTOREMANAGER)

if (ENABLE_TABSESSMANAGER)
	add_subdirectory (plugins/tabsessmanager)
endif (ENABLE_TABSESSMANAGER)

if (ENABLE_LASTFMSCROBBLE)
	add_subdirectory (plugins/lastfmscrobble)
endif (ENABLE_LASTFMSCROBBLE)

if (ENABLE_LAUGHTY)
	add_subdirectory (plugins/laughty)
endif (ENABLE_LAUGHTY)

if (ENABLE_LEMON)
	add_subdirectory (plugins/lemon)
endif (ENABLE_LEMON)

if (ENABLE_LHTR)
	add_subdirectory (plugins/lhtr)
endif (ENABLE_LHTR)

if (ENABLE_CEMENTO)
	add_subdirectory (plugins/cemento)
endif (ENABLE_CEMENTO)

if (ENABLE_GACTS)
	add_subdirectory (plugins/gacts)
endif (ENABLE_GACTS)

if (ENABLE_BLOGIQUE)
	add_subdirectory (plugins/blogique)
endif (ENABLE_BLOGIQUE)

if (ENABLE_MELLONETRAY)
	add_subdirectory (plugins/mellonetray)
endif (ENABLE_MELLONETRAY)

if (ENABLE_MONOCLE)
	add_subdirectory (plugins/monocle)
endif (ENABLE_MONOCLE)

if (ENABLE_PINTAB)
	add_subdirectory (plugins/pintab)
endif (ENABLE_PINTAB)

if (ENABLE_POGOOGLUE)
	add_subdirectory (plugins/pogooglue)
endif (ENABLE_POGOOGLUE)

if (ENABLE_OTLOZHU)
	add_subdirectory (plugins/otlozhu)
endif (ENABLE_OTLOZHU)

if (ENABLE_KBSWITCH)
	add_subdirectory (plugins/kbswitch)
endif (ENABLE_KBSWITCH)

if (ENABLE_XPROXY)
	add_subdirectory (plugins/xproxy)
endif (ENABLE_XPROXY)

if (ENABLE_DOLOZHEE)
	add_subdirectory (plugins/dolozhee)
endif (ENABLE_DOLOZHEE)

if (ENABLE_NACHEKU)
	add_subdirectory (plugins/nacheku)
endif (ENABLE_NACHEKU)

if (ENABLE_VROOBY)
	add_subdirectory (plugins/vrooby)
endif (ENABLE_VROOBY)

if (ENABLE_WOODPECKER)
	add_subdirectory (plugins/woodpecker)
endif (ENABLE_WOODPECKER)

if (ENABLE_XTAZY)
	add_subdirectory (plugins/xtazy)
endif (ENABLE_XTAZY)

if (ENABLE_DUMBEEP)
	add_subdirectory (plugins/dumbeep)
endif (ENABLE_DUMBEEP)

if (UNIX AND NOT APPLE)
	if (ENABLE_SHAITAN)
		add_subdirectory (plugins/shaitan)
	endif (ENABLE_SHAITAN)
endif (UNIX AND NOT APPLE)

if (UNIX AND NOT APPLE)
	if (ENABLE_LADS)
		add_subdirectory (plugins/lads)
	endif (ENABLE_LADS)
endif (UNIX AND NOT APPLE)

if (WIN32)
	if (ENABLE_Y7)
		add_subdirectory (plugins/y7)
	endif (ENABLE_Y7)
endif (WIN32)

if (APPLE)
	if (ENABLE_PIERRE)
		add_subdirectory (plugins/pierre)
	endif (ENABLE_PIERRE)
endif (APPLE)

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
		else (WIN32)
			message(SEND_ERROR "date not implemented")
			set(${RESULT} 000000)
		endif (WIN32)
	endif (NOT DEFINED LEECHCRAFT_VERSION)
	message (STATUS "Detected LeechCraft version: ${LEECHCRAFT_VERSION}")
endif (NOT DEFINED LEECHCRAFT_VERSION)

configure_file (config.h.in config.h)
configure_file (FindLeechCraft.cmake.in FindLeechCraft.cmake @ONLY)

add_subdirectory (util)
add_subdirectory (xmlsettingsdialog)
add_subdirectory (core)

if (UNIX)
	add_custom_target (data_leechcraft_gz ALL
		"${CMAKE_CURRENT_SOURCE_DIR}/data/compress"
		DEPENDS data/leechcraft.1
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
		)
endif (UNIX)

install (DIRECTORY util/ DESTINATION include/leechcraft/util/
	FILES_MATCHING PATTERN "*.h")
install (DIRECTORY interfaces/ DESTINATION include/leechcraft/interfaces/
	FILES_MATCHING PATTERN "*.h")
install (FILES xmlsettingsdialog/xmlsettingsdialog.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES xmlsettingsdialog/basesettingsmanager.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES xmlsettingsdialog/xsdconfig.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES xmlsettingsdialog/datasourceroles.h DESTINATION include/leechcraft/xmlsettingsdialog/)
install (FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/leechcraft/)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/FindLeechCraft.cmake DESTINATION ${LC_SHARE_DEST}/cmake/)
install (FILES InitLCPlugin.cmake DESTINATION share/cmake/Modules)

install (DIRECTORY share/leechcraft/ DESTINATION ${LC_SHARE_DEST})
install (DIRECTORY share/qml/common DESTINATION ${LC_QML_DEST}/org/LC)

install (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor)
if (UNIX AND NOT APPLE)
	install (DIRECTORY data/ DESTINATION share/man/man1 FILES_MATCHING PATTERN "*.gz")
	install (FILES freedesktop/leechcraft.desktop DESTINATION share/applications)
	install (FILES scripts/leechcraft-add-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
	install (FILES scripts/leechcraft-handle-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
endif (UNIX AND NOT APPLE)
