cmake_minimum_required(VERSION 2.6)
IF (COMMAND cmake_policy)
	cmake_policy (SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)
PROJECT (leechcraft)
#IF (NOT DISABLE_RPATH_TRICKS)
#	SET (CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#	SET (CMAKE_BUILD_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
#	SET (CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
#ENDIF (NOT DISABLE_RPATH_TRICKS)
IF (MSVC)
	SET (CMAKE_CXX_FLAGS "/EHa /W3")
	SET (CMAKE_CXX_FLAGS_RELEASE "/Ob2 /O2 /Ot /Oi /Oy /GT /GL /MD /D NDEBUG")
	SET (CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /GT /MDd")

	SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
	INCLUDE (LibHelperWin32Macros)
ENDIF (MSVC)
IF (WIN32)
	ADD_DEFINITIONS (-DWIN32_LEAN_AND_MEAN) # Don't include not needed headers with a lot of useless macross definitions
	ADD_DEFINITIONS (-D_WIN32_WINNT=0x0600) # Use Windows Vista definitions. Needed by Liznoo and KInotify
	ADD_DEFINITIONS (-DWINVER=0x0600)
	ADD_DEFINITIONS (-DNTDDI_VERSION=0x0600)
ENDIF (WIN32)
IF (UNIX OR (WIN32 AND MINGW))
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -std=c++0x")

	IF (NOT APPLE)
		SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
		SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
	ENDIF (NOT APPLE)

	IF (EXTRA_WARNINGS)
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
	ENDIF (EXTRA_WARNINGS)

	IF (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND ENABLE_CLANG_QUIRKS)
		MESSAGE (STATUS "Enable Clang-specific quirks for Qt...")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_RVALUE_REFS -DQ_COMPILER_DECLTYPE")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_VARIADIC_TEMPLATES -DQ_COMPILER_AUTO_TYPE -DQ_COMPILER_EXTERN_TEMPLATES -DQ_COMPILER_DEFAULT_DELETE_MEMBERS -DQ_COMPILER_CLASS_ENUM -DQ_COMPILER_INITIALIZER_LISTS")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_LAMBDA -DQ_COMPILER_UNICODE_STRINGS")
		SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQ_COMPILER_CONSTEXPR")
	ENDIF (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") AND ENABLE_CLANG_QUIRKS)
ENDIF (UNIX OR (WIN32 AND MINGW))

OPTION (USE_QT5 "Enable Qt5 build" OFF)

IF (GPTOOLS_CPU AND GPTOOLS_MEM)
	SET (GPTOOLS_CPU FALSE)
	SET (GPTOOLS_MEM FALSE)
	SET (GPTOOLS_CPU_AND_MEM TRUE)
ENDIF (GPTOOLS_CPU AND GPTOOLS_MEM)
IF (GPTOOLS_MEM)
	MESSAGE (STATUS "Enabled memory profiling via Google perftools")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc")
ENDIF (GPTOOLS_MEM)
IF (GPTOOLS_CPU)
	MESSAGE (STATUS "Enabled CPU profiling via Google perftools")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
ENDIF (GPTOOLS_CPU)
IF (GPTOOLS_CPU_AND_MEM)
	MESSAGE (STATUS "Enabled both CPU and memory profiling via Google perftools")
	SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc_and_profiler")
	SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc_and_profiler")
ENDIF (GPTOOLS_CPU_AND_MEM)

IF (UNIX AND NOT APPLE)
	IF (NOT LIBDIR)
		SET (LIBDIR "lib${LIB_SUFFIX}")
	ENDIF (NOT LIBDIR)

	SET (LC_BINDIR "bin")
	SET (LC_PLUGINS_DEST "${LIBDIR}/leechcraft/plugins")
	SET (LC_TRANSLATIONS_DEST "share/leechcraft/translations")
	SET (LC_SETTINGS_DEST "share/leechcraft/settings")
	SET (LC_QML_DEST "share/leechcraft/qml")
	SET (LC_SHARE_DEST "share/leechcraft")
	SET (LC_INSTALLEDMANIFEST_DEST "share/leechcraft/installed")
	SET (LC_GLOBAL_SETTINGS_DEST "share/leechcraft/rc")
	SET (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
ELSEIF (WIN32)
	IF (NOT LIBDIR)
		SET (LIBDIR "${CMAKE_INSTALL_PREFIX}")
	ENDIF (NOT LIBDIR)

	SET (LC_BINDIR "${CMAKE_INSTALL_PREFIX}")
	SET (LC_PLUGINS_DEST "plugins/bin")
	SET (LC_TRANSLATIONS_DEST "translations")
	SET (LC_SETTINGS_DEST "settings")
	SET (LC_QML_DEST "share/qml")
	SET (LC_SHARE_DEST "share")
	SET (LC_INSTALLEDMANIFEST_DEST "share/installed")
	SET (LC_GLOBAL_SETTINGS_DEST "settings")
	SET (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
ELSE (UNIX AND NOT APPLE)
	SET (LIBDIR "lib")
	SET (LC_BINDIR "leechcraft.app/Contents/MacOs")
	SET (LC_PLUGINS_DEST "leechcraft.app/Contents/PlugIns")
	SET (LC_TRANSLATIONS_DEST "leechcraft.app/Contents/Resources/translations")
	SET (LC_SETTINGS_DEST "leechcraft.app/Contents/Resources/settings")
	SET (LC_QML_DEST "leechcraft.app/Contents/Resources/share/qml")
	SET (LC_SHARE_DEST "leechcraft.app/Contents/Resources/share")
	SET (LC_INSTALLEDMANIFEST_DEST "leechcraft.app/Contents/Resources/share/installed")
	SET (LC_GLOBAL_SETTINGS_DEST "leechcraft.app/Contents/Resources/share/rc")
	SET (LC_GLOBAL_SETTINGS_DEST_IN "${CMAKE_INSTALL_PREFIX}/${LC_GLOBAL_SETTINGS_DEST}")
ENDIF (UNIX AND NOT APPLE)

SET (Boost_ADDITIONAL_VERSIONS "1.47" "1.46" "1.46.1" "1.44" "1.44.0")
FIND_PACKAGE (Boost REQUIRED COMPONENTS date_time filesystem program_options system thread)

IF (USE_QT5)
	SET (CMAKE_INCLUDE_CURRENT_DIR ON)
	FIND_PACKAGE (Qt5Widgets)
	ADD_DEFINITIONS (-DUSE_QT5)
ELSE (USE_QT5)
	FIND_PACKAGE (Qt4 REQUIRED)
ENDIF (USE_QT5)

STRING (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr" STANDARD_PREFIX)
IF (NOT STANDARD_PREFIX)
	STRING (COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "/usr/local" STANDARD_PREFIX)
ENDIF ()

IF (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)
	ADD_DEFINITIONS (-DINSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
ENDIF (CMAKE_INSTALL_PREFIX AND NOT STANDARD_PREFIX)

IF (NOT CMAKE_VERSION VERSION_LESS 2.8.6)
	MESSAGE (STATUS "CMake 2.8.6 or greater found, enabling automoc...")
	SET (CMAKE_AUTOMOC TRUE)
	SET (LC_NO_MOC TRUE)
ENDIF (NOT CMAKE_VERSION VERSION_LESS 2.8.6)

SET (LEECHCRAFT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
SET (LEECHCRAFT_LIBRARIES lcutil xmlsettingsdialog)

# Various options
OPTION (STRICT_LICENSING "Makes LeechCraft more prepared, for example, to lacking artwork due to licensing restrictions" OFF)

IF (STRICT_LICENSING)
	ADD_DEFINITIONS (-DSTRICT_LICENSING)
ENDIF (STRICT_LICENSING)

OPTION (WITH_PLUGINS "Build a somewhat standard set of plugins as well" ON)

# Basic & traditional set of options
OPTION (ENABLE_ADVANCEDNOTIFICATIONS "Build Advanced Notifications module for more customizable notifications" ${WITH_PLUGINS})
OPTION (ENABLE_AGGREGATOR "Build Aggregator, the RSS/Atom feed reader" ${WITH_PLUGINS})
OPTION (ENABLE_ANHERO "Build AnHero, the crash handler" ${WITH_PLUGINS})
OPTION (ENABLE_AUSCRIE "Build Auscrie, the auto screenshooter" ${WITH_PLUGINS})
OPTION (ENABLE_AZOTH "Build Azoth, the IM client" ${WITH_PLUGINS})
OPTION (ENABLE_DBUSMANAGER "Build DBusManager, the D-Bus support for the LeechCraft" ${WITH_PLUGINS})
OPTION (ENABLE_DEADLYRICS "Build DeadLyrics, the client for the lyricwiki.org site" ${WITH_PLUGINS})
OPTION (ENABLE_GLANCE "Quick overview of tabs" ${WITH_PLUGINS})
OPTION (ENABLE_GMAILNOTIFIER "Build GMail Notifier which notifies about new mail in your GMail inbox" ${WITH_PLUGINS})
OPTION (ENABLE_HISTORYHOLDER "Build HistoryHolder, the keeper of the history" ${WITH_PLUGINS})
OPTION (ENABLE_HTTP "Build CSTP, the HTTP implementation" ${WITH_PLUGINS})
OPTION (ENABLE_KINOTIFY "Build Kinotify, fancy kinetic notifications" ${WITH_PLUGINS})
OPTION (ENABLE_KNOWHOW "Build KnowHow for displaying tips of the day" ${WITH_PLUGINS})
OPTION (ENABLE_LMP "Build LMP, the LeechCraft Music Player" ${WITH_PLUGINS})
OPTION (ENABLE_NETWORKMONITOR "Build NetworkMonitor, the monitor for HTTP requests" ${WITH_PLUGINS})
OPTION (ENABLE_NEWLIFE "Build NewLife, the settings importer" ${WITH_PLUGINS})
OPTION (ENABLE_PINTAB "Build Pintab for pinning important tabs" ${WITH_PLUGINS})
OPTION (ENABLE_POGOOGLUE "Build Pogooglue, Google search client" ${WITH_PLUGINS})
OPTION (ENABLE_POSHUKU "Build Poshuku, the WWW Browser" ${WITH_PLUGINS})
OPTION (ENABLE_SECMAN "Build SecMan, security & personal data manager" ${WITH_PLUGINS})
OPTION (ENABLE_SEEKTHRU "Build SeekThru, the client for the OpenSearch-aware web sites" ${WITH_PLUGINS})
OPTION (ENABLE_SUMMARY "Build Summary, the plugin that shows a handy summary of the stuff going on" ${WITH_PLUGINS})
OPTION (ENABLE_SYNCER "Build Syncer, the synchronization plugin for LeechCraft" ${WITH_PLUGINS})
OPTION (ENABLE_TABSESSMANAGER "Build Tab Session Manager" ${WITH_PLUGINS})
OPTION (ENABLE_TABSLIST "Quick overview of tabs" ${WITH_PLUGINS})
OPTION (ENABLE_TORRENT "Build BitTorrent, the BitTorrent client" ${WITH_PLUGINS})
OPTION (ENABLE_VGRABBER "Build vGrabber, the vkontakte.ru music/video getter" ${WITH_PLUGINS})

# Traditional, but dependent on 3dparty
OPTION (ENABLE_POPISHU "Build Popishu, a simple QScintilla-based text editor" OFF)
OPTION (ENABLE_QROSP "Build Qrosp, the scripting support plugin" OFF)

# New
OPTION (ENABLE_BLOGIQUE "Build Blogique, LeechCraft blogging client" ${WITH_PLUGINS})
OPTION (ENABLE_DOLOZHEE "Build Dolozhee, the feature request and bug reporter" ${WITH_PLUGINS})
OPTION (ENABLE_GACTS "Build Global Actions provider plugin" ${WITH_PLUGINS})
OPTION (ENABLE_HOTSENSORS "Build HotSensors, temperature sensors quark" ${WITH_PLUGINS})
OPTION (ENABLE_HOTSTREAMS "Build HotStreams, radio streams provider module" ${WITH_PLUGINS})
OPTION (ENABLE_KBSWITCH "Build KBSwitch, a keyboard plugin" ${WITH_PLUGINS})
OPTION (ENABLE_KRIGSTASK "Build Krigstask, applications switcher" ${WITH_PLUGINS})
OPTION (ENABLE_LACKMAN "Build LackMan, the package manager for LeechCraft" ${WITH_PLUGINS})
OPTION (ENABLE_LADS "Build Lads, Ubuntu Unity integration layer" ${WITH_PLUGINS})
OPTION (ENABLE_LASTFMSCROBBLE "Build Last.fm scrobbler" ${WITH_PLUGINS})
OPTION (ENABLE_LAUGHTY "Build Desktop Notifications server" ${WITH_PLUGINS})
OPTION (ENABLE_LEMON "Build LeechCraft Network Monitor" ${WITH_PLUGINS})
OPTION (ENABLE_LHTR "Build LeechCraft HTML Text editoR" ${WITH_PLUGINS})
OPTION (ENABLE_LIZNOO "Build Liznoo, our nice power manager" ${WITH_PLUGINS})
OPTION (ENABLE_MONOCLE "Build Monocle, modular document viewer for LeechCraft" ${WITH_PLUGINS})
OPTION (ENABLE_MUSICZOMBIE "Build MusicZombie, MusicBrainz.org client" ${WITH_PLUGINS})
OPTION (ENABLE_NACHEKU "Build Nacheku, clipboard/download dir watcher" ${WITH_PLUGINS})
OPTION (ENABLE_NETSTOREMANAGER "Build NetStoreManager for managing network data storages like Yandex.Disk" ${WITH_PLUGINS})
OPTION (ENABLE_OTLOZHU "Build Otlozhu, a GTD-inspired ToDo manager" ${WITH_PLUGINS})
OPTION (ENABLE_POLEEMERY "Build Poleemery, the personal finances manager" ${WITH_PLUGINS})
OPTION (ENABLE_SB2 "Build SB2, next-gen fluid sidebar" ${WITH_PLUGINS})
OPTION (ENABLE_SHAITAN "Build Shaitan, Embedded terminal emulator" ${WITH_PLUGINS})
OPTION (ENABLE_SYSNOTIFY "Build Sysnotify for using D-Bus notification API" ${WITH_PLUGINS})
OPTION (ENABLE_TOUCHSTREAMS "Build TouchStreams, VK.com music streaming plugin" ${WITH_PLUGINS})
OPTION (ENABLE_TPI "Build Task Progress Indicator quark plugin" ${WITH_PLUGINS})
OPTION (ENABLE_VROOBY "Build Vrooby, removable storage devices manager" ${WITH_PLUGINS})
OPTION (ENABLE_XPROXY "Build XProxy, the advanced proxy manager" ${WITH_PLUGINS})
OPTION (ENABLE_XTAZY "Build Xtazy, plugin for keeping track of a currently playing song" ${WITH_PLUGINS})

OPTION (ENABLE_BLACKDASH "Build BlackDash, the dashboard plugin for LeechCraft" OFF)
OPTION (ENABLE_CHOROID "Build Choroid, image viewer" OFF)
OPTION (ENABLE_SNAILS "Build Snails, LeechCraft mail client" OFF)
OPTION (ENABLE_TWIFEE "Build Twifee, the Twitter client" OFF)
OPTION (ENABLE_CEMENTO "Build Cemento, LeechCraft common persistent storage" OFF)

# Dumb alternatives
OPTION (ENABLE_DUMBEEP "Build Dumbeep, a dumb sound notifier for those fearing true LMP power" OFF)

IF (WIN32)
	OPTION (ENABLE_Y7 "Build Y7, Windows 7 integration plugin" ${WITH_PLUGINS})
ENDIF (WIN32)

IF (UNIX)
	OPTION (ENABLE_LAUNCHY "Build Launchy, third-party application launcher module" ${WITH_PLUGINS})
ENDIF (UNIX)

IF (APPLE)
	OPTION (ENABLE_PIERRE "Build Pierre, Mac OS X integration layer" ${WITH_PLUGINS})
ENDIF (APPLE)

# Old ones
OPTION (ENABLE_SHELLOPEN "Build ShellOpen, the plugin supporting handling entities via external applications" OFF)

# Defunct
OPTION (ENABLE_JUFFED "Build JuffEd, the advanced text editor" OFF)
OPTION (ENABLE_OTZERKALU "Build Otzerkalu, which allows one to download web sites recursively" OFF)
OPTION (ENABLE_LAURE "Build Laure, the new generation audio/video player" OFF)

ENABLE_TESTING ()

FUNCTION (CreateTrs PlugName Langs CompiledTranVar)
	STRING (LENGTH "${PlugName}" _pnameLength)
	SET (_basename "leechcraft")
	IF (_pnameLength)
		SET (_basename "${_basename}_${PlugName}")
	ENDIF (_pnameLength)
	FOREACH (_LANG ${Langs})
		SET (_tran ${_tran} ${_basename}_${_LANG}.ts)
		SET (_ctran ${_ctran} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}_${_LANG}.qm)
	ENDFOREACH (_LANG)
	SET (${CompiledTranVar} ${_ctran} PARENT_SCOPE)

	ADD_CUSTOM_COMMAND (OUTPUT ${_ctran}
		COMMAND "${QT_LRELEASE_EXECUTABLE}" ${_tran}
		DEPENDS ${_tran}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	INSTALL (FILES ${_ctran} DESTINATION ${LC_TRANSLATIONS_DEST})
	SET (${CompiledTranVar} ${_ctran} PARENT_SCOPE)
ENDFUNCTION (CreateTrs)

SET (GENCPP_XML_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../tools/scripts/translationstuff/gencpp.sh")

FUNCTION (CreateTrsUpTarget PlugName Langs Sources Forms Xml)
	STRING (LENGTH "${PlugName}" _pnameLength)
	STRING (LENGTH "${Xml}" _xmlLength)
	SET (_basename "leechcraft")
	IF (_pnameLength)
		SET (_basename "${_basename}_${PlugName}")
	ENDIF (_pnameLength)
	IF (_xmlLength)
		FOREACH (_LANG ${Langs})
			ADD_CUSTOM_TARGET (${_basename}_ts_${_LANG}
					${GENCPP_XML_PATH} ${Xml}
					COMMAND ${QT_LUPDATE_EXECUTABLE} "-noobsolete" ${Sources} ${Forms} "dummy.cpp" "-ts" "${_basename}_${_LANG}.ts"
					COMMAND "rm" "-f" "dummy.cpp"
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
		ENDFOREACH (_LANG)
	ELSE (_xmlLength)
		FOREACH (_LANG ${Langs})
			ADD_CUSTOM_TARGET (${_basename}_ts_${_LANG}
					${QT_LUPDATE_EXECUTABLE} "-noobsolete" ${Sources} ${Forms} "-ts" "${_basename}_${_LANG}.ts"
					WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
		ENDFOREACH (_LANG)
	ENDIF (_xmlLength)
ENDFUNCTION (CreateTrsUpTarget)

macro (QtWrapUi outfiles)
	if (USE_QT5)
		QT5_WRAP_UI (UIS_H ${ARGN})
	else ()
		QT4_WRAP_UI (UIS_H ${ARGN})
	endif ()
	set (${outfiles} ${${outfiles}} ${UIS_H})
endmacro ()

macro (QtAddResources outfiles)
	if (USE_QT5)
		QT5_ADD_RESOURCES (RCCS ${ARGN})
	else ()
		QT4_ADD_RESOURCES (RCCS ${ARGN})
	endif ()
	set (${outfiles} ${${outfiles}} ${RCCS})
endmacro ()

macro (FindQtLibs Target)
	if (USE_QT5)
		QT5_USE_MODULES (${Target} ${ARGN})
	else ()
		foreach (_currentArg ${ARGN})
			if ((NOT ${_currentArg} STREQUAL Concurrent) AND
					(NOT ${_currentArg} STREQUAL Widgets))
				if (${_currentArg} STREQUAL Quick)
					set (QT_USE_QTDECLARATIVE TRUE)
				else ()
					string (TOUPPER ${_currentArg} _currentArgUpper)
					set (QT_USE_QT${_currentArgUpper} TRUE)
				endif ()
			endif ()
			include (${QT_USE_FILE})
		endforeach()
	endif ()
endmacro()

IF (ENABLE_SUMMARY)
	ADD_SUBDIRECTORY (plugins/summary)
ENDIF (ENABLE_SUMMARY)

IF (ENABLE_TORRENT)
	ADD_SUBDIRECTORY (plugins/bittorrent)
ENDIF (ENABLE_TORRENT)

IF (ENABLE_AGGREGATOR)
	ADD_SUBDIRECTORY (plugins/aggregator)
ENDIF (ENABLE_AGGREGATOR)

IF (ENABLE_CHOROID)
	ADD_SUBDIRECTORY (plugins/choroid)
ENDIF (ENABLE_CHOROID)

IF (ENABLE_POSHUKU)
	ADD_SUBDIRECTORY (plugins/poshuku)
ENDIF (ENABLE_POSHUKU)

IF (ENABLE_HTTP)
	ADD_SUBDIRECTORY (plugins/cstp)
ENDIF (ENABLE_HTTP)

IF (ENABLE_LMP)
	ADD_SUBDIRECTORY (plugins/lmp)
ENDIF (ENABLE_LMP)

IF (ENABLE_DBUSMANAGER)
	ADD_SUBDIRECTORY (plugins/dbusmanager)
ENDIF (ENABLE_DBUSMANAGER)

IF (ENABLE_DEADLYRICS)
	ADD_SUBDIRECTORY (plugins/deadlyrics)
ENDIF (ENABLE_DEADLYRICS)

IF (ENABLE_SEEKTHRU)
	ADD_SUBDIRECTORY (plugins/seekthru)
ENDIF (ENABLE_SEEKTHRU)

IF (ENABLE_NETWORKMONITOR)
	ADD_SUBDIRECTORY (plugins/networkmonitor)
ENDIF (ENABLE_NETWORKMONITOR)

IF (ENABLE_HISTORYHOLDER)
	ADD_SUBDIRECTORY (plugins/historyholder)
ENDIF (ENABLE_HISTORYHOLDER)

IF (ENABLE_NEWLIFE)
	ADD_SUBDIRECTORY (plugins/newlife)
ENDIF (ENABLE_NEWLIFE)

IF (ENABLE_ANHERO)
	ADD_SUBDIRECTORY (plugins/anhero)
ENDIF (ENABLE_ANHERO)

IF (ENABLE_VGRABBER)
	ADD_SUBDIRECTORY (plugins/vgrabber)
ENDIF (ENABLE_VGRABBER)

IF (ENABLE_KINOTIFY)
	ADD_SUBDIRECTORY (plugins/kinotify)
ENDIF (ENABLE_KINOTIFY)

IF (ENABLE_AUSCRIE)
	ADD_SUBDIRECTORY (plugins/auscrie)
ENDIF (ENABLE_AUSCRIE)

IF (ENABLE_AZOTH)
	ADD_SUBDIRECTORY (plugins/azoth)
ENDIF (ENABLE_AZOTH)

IF (ENABLE_SECMAN)
	ADD_SUBDIRECTORY (plugins/secman)
ENDIF (ENABLE_SECMAN)

IF (ENABLE_QROSP)
	ADD_SUBDIRECTORY (plugins/qrosp)
ENDIF (ENABLE_QROSP)

IF (ENABLE_POPISHU)
	ADD_SUBDIRECTORY (plugins/popishu)
ENDIF (ENABLE_POPISHU)

IF (ENABLE_HOTSENSORS)
	ADD_SUBDIRECTORY (plugins/hotsensors)
ENDIF (ENABLE_HOTSENSORS)

IF (ENABLE_HOTSTREAMS)
	ADD_SUBDIRECTORY (plugins/hotstreams)
ENDIF (ENABLE_HOTSTREAMS)

IF (ENABLE_KRIGSTASK)
	ADD_SUBDIRECTORY (plugins/krigstask)
ENDIF (ENABLE_KRIGSTASK)

IF (ENABLE_LACKMAN)
	ADD_SUBDIRECTORY (plugins/lackman)
ENDIF (ENABLE_LACKMAN)

IF (ENABLE_LAUNCHY)
	ADD_SUBDIRECTORY (plugins/launchy)
ENDIF (ENABLE_LAUNCHY)

IF (ENABLE_SYNCER)
	ADD_SUBDIRECTORY (plugins/syncer)
ENDIF (ENABLE_SYNCER)

IF (ENABLE_SHELLOPEN)
	ADD_SUBDIRECTORY (plugins/shellopen)
ENDIF (ENABLE_SHELLOPEN)

IF (ENABLE_LAURE)
	ADD_SUBDIRECTORY (plugins/laure)
ENDIF (ENABLE_LAURE)

IF (ENABLE_JUFFED)
	ADD_SUBDIRECTORY (plugins/juffed)
ENDIF (ENABLE_JUFFED)

IF (ENABLE_GMAILNOTIFIER)
	ADD_SUBDIRECTORY (plugins/gmailnotifier)
ENDIF (ENABLE_GMAILNOTIFIER)

IF (ENABLE_OTZERKALU)
	ADD_SUBDIRECTORY (plugins/otzerkalu)
ENDIF (ENABLE_OTZERKALU)

IF (ENABLE_ADVANCEDNOTIFICATIONS)
	ADD_SUBDIRECTORY (plugins/advancednotifications)
ENDIF (ENABLE_ADVANCEDNOTIFICATIONS)

IF (ENABLE_GLANCE)
	ADD_SUBDIRECTORY (plugins/glance)
ENDIF (ENABLE_GLANCE)

IF (ENABLE_TABSLIST)
	ADD_SUBDIRECTORY (plugins/tabslist)
ENDIF (ENABLE_TABSLIST)

IF (ENABLE_BLACKDASH)
	ADD_SUBDIRECTORY (plugins/blackdash)
ENDIF (ENABLE_BLACKDASH)

IF (ENABLE_KNOWHOW)
	ADD_SUBDIRECTORY (plugins/knowhow)
ENDIF (ENABLE_KNOWHOW)

IF (ENABLE_POLEEMERY)
	ADD_SUBDIRECTORY (plugins/poleemery)
ENDIF (ENABLE_POLEEMERY)

IF (ENABLE_SB2)
	ADD_SUBDIRECTORY (plugins/sb2)
ENDIF (ENABLE_SB2)

IF (ENABLE_SNAILS)
	ADD_SUBDIRECTORY (plugins/snails)
ENDIF (ENABLE_SNAILS)

IF (ENABLE_SYSNOTIFY)
	ADD_SUBDIRECTORY (plugins/sysnotify)
ENDIF (ENABLE_SYSNOTIFY)

IF (ENABLE_TOUCHSTREAMS)
	ADD_SUBDIRECTORY (plugins/touchstreams)
ENDIF (ENABLE_TOUCHSTREAMS)

IF (ENABLE_TPI)
	ADD_SUBDIRECTORY (plugins/tpi)
ENDIF (ENABLE_TPI)

IF (ENABLE_TWIFEE)
	ADD_SUBDIRECTORY (plugins/twifee)
ENDIF (ENABLE_TWIFEE)

IF (ENABLE_LIZNOO)
	ADD_SUBDIRECTORY (plugins/liznoo)
ENDIF (ENABLE_LIZNOO)

IF (ENABLE_MUSICZOMBIE)
	ADD_SUBDIRECTORY (plugins/musiczombie)
ENDIF (ENABLE_MUSICZOMBIE)

IF (ENABLE_NETSTOREMANAGER)
	ADD_SUBDIRECTORY (plugins/netstoremanager)
ENDIF (ENABLE_NETSTOREMANAGER)

IF (ENABLE_TABSESSMANAGER)
	ADD_SUBDIRECTORY (plugins/tabsessmanager)
ENDIF (ENABLE_TABSESSMANAGER)

IF (ENABLE_LASTFMSCROBBLE)
	ADD_SUBDIRECTORY (plugins/lastfmscrobble)
ENDIF (ENABLE_LASTFMSCROBBLE)

IF (ENABLE_LAUGHTY)
	ADD_SUBDIRECTORY (plugins/laughty)
ENDIF (ENABLE_LAUGHTY)

IF (ENABLE_LEMON)
	ADD_SUBDIRECTORY (plugins/lemon)
ENDIF (ENABLE_LEMON)

IF (ENABLE_LHTR)
	ADD_SUBDIRECTORY (plugins/lhtr)
ENDIF (ENABLE_LHTR)

IF (ENABLE_CEMENTO)
	ADD_SUBDIRECTORY (plugins/cemento)
ENDIF (ENABLE_CEMENTO)

IF (ENABLE_GACTS)
	ADD_SUBDIRECTORY (plugins/gacts)
ENDIF (ENABLE_GACTS)

IF (ENABLE_BLOGIQUE)
	ADD_SUBDIRECTORY (plugins/blogique)
ENDIF (ENABLE_BLOGIQUE)

IF (ENABLE_MONOCLE)
	ADD_SUBDIRECTORY (plugins/monocle)
ENDIF (ENABLE_MONOCLE)

IF (ENABLE_PINTAB)
	ADD_SUBDIRECTORY (plugins/pintab)
ENDIF (ENABLE_PINTAB)

IF (ENABLE_POGOOGLUE)
	ADD_SUBDIRECTORY (plugins/pogooglue)
ENDIF (ENABLE_POGOOGLUE)

IF (ENABLE_OTLOZHU)
	ADD_SUBDIRECTORY (plugins/otlozhu)
ENDIF (ENABLE_OTLOZHU)

IF (ENABLE_KBSWITCH)
	ADD_SUBDIRECTORY (plugins/kbswitch)
ENDIF (ENABLE_KBSWITCH)

IF (ENABLE_XPROXY)
	ADD_SUBDIRECTORY (plugins/xproxy)
ENDIF (ENABLE_XPROXY)

IF (ENABLE_DOLOZHEE)
	ADD_SUBDIRECTORY (plugins/dolozhee)
ENDIF (ENABLE_DOLOZHEE)

IF (ENABLE_NACHEKU)
	ADD_SUBDIRECTORY (plugins/nacheku)
ENDIF (ENABLE_NACHEKU)

IF (ENABLE_VROOBY)
	ADD_SUBDIRECTORY (plugins/vrooby)
ENDIF (ENABLE_VROOBY)

IF (ENABLE_XTAZY)
	ADD_SUBDIRECTORY (plugins/xtazy)
ENDIF (ENABLE_XTAZY)

IF (ENABLE_DUMBEEP)
	ADD_SUBDIRECTORY (plugins/dumbeep)
ENDIF (ENABLE_DUMBEEP)

IF (UNIX AND NOT APPLE)
	IF (ENABLE_SHAITAN)
		ADD_SUBDIRECTORY (plugins/shaitan)
	ENDIF (ENABLE_SHAITAN)
ENDIF (UNIX AND NOT APPLE)

IF (UNIX AND NOT APPLE)
	IF (ENABLE_LADS)
		ADD_SUBDIRECTORY (plugins/lads)
	ENDIF (ENABLE_LADS)
ENDIF (UNIX AND NOT APPLE)

IF (WIN32)
	IF (ENABLE_Y7)
		ADD_SUBDIRECTORY (plugins/y7)
	ENDIF (ENABLE_Y7)
ENDIF (WIN32)

IF (APPLE)
	IF (ENABLE_PIERRE)
		ADD_SUBDIRECTORY (plugins/pierre)
	ENDIF (ENABLE_PIERRE)
ENDIF (APPLE)

IF (NOT DEFINED LEECHCRAFT_VERSION)
	FIND_PROGRAM (GIT_CMD git)
	EXECUTE_PROCESS (COMMAND "${GIT_CMD}" "--git-dir=${CMAKE_CURRENT_SOURCE_DIR}/../.git" describe
		OUTPUT_VARIABLE LEECHCRAFT_VERSION
		OUTPUT_STRIP_TRAILING_WHITESPACE
		)
	IF (NOT DEFINED LEECHCRAFT_VERSION)
		IF (WIN32)
			EXECUTE_PROCESS(COMMAND "date" "/T" OUTPUT_VARIABLE
				${LEECHCRAFT_VERSION})
			STRING(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
				${LEECHCRAFT_VERSION} ${${LEECHCRAFT_VERSION}})
		ELSEIF(UNIX)
			EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE
				${LEECHCRAFT_VERSION})
			STRING(REGEX REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1"
				${LEECHCRAFT_VERSION} ${${LEECHCRAFT_VERSION}})
		ELSE (WIN32)
			MESSAGE(SEND_ERROR "date not implemented")
			SET(${RESULT} 000000)
		ENDIF (WIN32)
	ENDIF (NOT DEFINED LEECHCRAFT_VERSION)
	MESSAGE (STATUS "Detected LeechCraft version: ${LEECHCRAFT_VERSION}")
ENDIF (NOT DEFINED LEECHCRAFT_VERSION)

CONFIGURE_FILE (config.h.in config.h)
CONFIGURE_FILE (FindLeechCraft.cmake.in FindLeechCraft.cmake @ONLY)

ADD_SUBDIRECTORY (util)
ADD_SUBDIRECTORY (xmlsettingsdialog)
ADD_SUBDIRECTORY (core)

IF (UNIX)
	ADD_CUSTOM_TARGET (data_leechcraft_gz ALL
		"${CMAKE_CURRENT_SOURCE_DIR}/data/compress"
		DEPENDS data/leechcraft.1
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
		)
ENDIF (UNIX)

INSTALL (DIRECTORY util/ DESTINATION include/leechcraft/util/
	FILES_MATCHING PATTERN "*.h")
INSTALL (DIRECTORY interfaces/ DESTINATION include/leechcraft/interfaces/
	FILES_MATCHING PATTERN "*.h")
INSTALL (FILES xmlsettingsdialog/xmlsettingsdialog.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES xmlsettingsdialog/basesettingsmanager.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES xmlsettingsdialog/xsdconfig.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES xmlsettingsdialog/datasourceroles.h DESTINATION include/leechcraft/xmlsettingsdialog/)
INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/config.h DESTINATION include/leechcraft/)

INSTALL (FILES ${CMAKE_CURRENT_BINARY_DIR}/FindLeechCraft.cmake DESTINATION ${LC_SHARE_DEST}/cmake/)
INSTALL (FILES InitLCPlugin.cmake DESTINATION share/cmake/Modules)

INSTALL (DIRECTORY share/leechcraft/ DESTINATION ${LC_SHARE_DEST})
INSTALL (DIRECTORY share/qml/common DESTINATION ${LC_QML_DEST}/org/LC)

INSTALL (DIRECTORY freedesktop/icons/ DESTINATION share/icons/hicolor)
IF (UNIX AND NOT APPLE)
	INSTALL (DIRECTORY data/ DESTINATION share/man/man1 FILES_MATCHING PATTERN "*.gz")
	INSTALL (FILES freedesktop/leechcraft.desktop DESTINATION share/applications)
	INSTALL (FILES scripts/leechcraft-add-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
	INSTALL (FILES scripts/leechcraft-handle-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
ENDIF (UNIX AND NOT APPLE)
