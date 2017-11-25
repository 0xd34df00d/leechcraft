cmake_minimum_required(VERSION 3.1)
cmake_policy (SET CMP0042 NEW)
cmake_policy (SET CMP0071 NEW)

project (leechcraft)

set (CMAKE_MACOSX_RPATH OFF)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED TRUE)
set (CMAKE_CXX_EXTENSIONS OFF)

option (EXTRA_WARNINGS "Enable extra compiler warnings" OFF)

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
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -Wall")

	add_definitions (-DQT_MESSAGELOGCONTEXT)

	if (NOT APPLE)
		set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--as-needed")
		set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed")
	endif ()

	if (EXTRA_WARNINGS)
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
	endif ()

	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-register")
	endif ()
endif ()

include (CMakeDependentOption)

if (GPTOOLS_CPU AND GPTOOLS_MEM)
	set (GPTOOLS_CPU FALSE)
	set (GPTOOLS_MEM FALSE)
	set (GPTOOLS_CPU_AND_MEM TRUE)
endif ()
if (GPTOOLS_MEM)
	message (STATUS "Enabled memory profiling via Google perftools")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc")
endif ()
if (GPTOOLS_CPU)
	message (STATUS "Enabled CPU profiling via Google perftools")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lprofiler")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -lprofiler")
endif ()
if (GPTOOLS_CPU_AND_MEM)
	message (STATUS "Enabled both CPU and memory profiling via Google perftools")
	set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc_and_profiler")
	set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc_and_profiler")
endif ()

option (USE_UNIX_LAYOUT "Install LeechCraft with a UNIX layout on OS X" OFF)

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
	QT5_ADD_RESOURCES (RCCS ${ARGN})
	set (${outfiles} ${${outfiles}} ${RCCS})
endmacro ()

macro (FindQtLibs Target)
	cmake_policy (SET CMP0043 NEW)
	set (CMAKE_INCLUDE_CURRENT_DIR ON)
	find_package (Qt5Widgets)
	QT5_USE_MODULES (${Target} ${ARGN})
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

# Basic & traditional set of options
option (ENABLE_ADVANCEDNOTIFICATIONS "Build Advanced Notifications module for more customizable notifications" ${WITH_PLUGINS})
option (ENABLE_AGGREGATOR "Build Aggregator, the RSS/Atom feed reader" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_ANHERO "Build AnHero, the crash handler" ${WITH_PLUGINS} "NOT WIN32" OFF)
option (ENABLE_AUSCRIE "Build Auscrie, the auto screenshooter" ${WITH_PLUGINS})
option (ENABLE_AZOTH "Build Azoth, the IM client" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_DBUSMANAGER "Build DBusManager, the D-Bus support for the LeechCraft" ${WITH_PLUGINS} "NOT WIN32" OFF)
option (ENABLE_DEADLYRICS "Build DeadLyrics, the client for the lyricwiki.org site" ${WITH_PLUGINS})
option (ENABLE_GLANCE "Quick overview of tabs" ${WITH_PLUGINS})
option (ENABLE_GMAILNOTIFIER "Build GMail Notifier which notifies about new mail in your GMail inbox" ${WITH_PLUGINS})
option (ENABLE_HISTORYHOLDER "Build HistoryHolder, the keeper of the history" ${WITH_PLUGINS})
option (ENABLE_HTTP "Build CSTP, the HTTP implementation" ${WITH_PLUGINS})
option (ENABLE_HTTHARE "Build HttThare for sharing local documents via HTTP" ${WITH_PLUGINS})
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
option (ENABLE_SYNCER "Build Syncer, the synchronization plugin for LeechCraft" FALSE)
option (ENABLE_TABSESSMANAGER "Build Tab Session Manager" ${WITH_PLUGINS})
option (ENABLE_TABSLIST "Quick overview of tabs" ${WITH_PLUGINS})
option (ENABLE_TORRENT "Build BitTorrent, the BitTorrent client" ${WITH_PLUGINS})
option (ENABLE_VGRABBER "Build vGrabber, the vkontakte.ru music/video getter" ${WITH_PLUGINS})

# Traditional, but dependent on 3dparty
option (ENABLE_POPISHU "Build Popishu, a simple QScintilla-based text editor" OFF)
option (ENABLE_QROSP "Build Qrosp, the scripting support plugin" OFF)

# New
option (ENABLE_BLASQ "Build Blasq, client for cloud image storages like Picasa or Flickr" ${WITH_PLUGINS})
option (ENABLE_BLOGIQUE "Build Blogique, LeechCraft blogging client" ${WITH_PLUGINS})
option (ENABLE_CERTMGR "Build CertMgr for managing SSL certificates" ${WITH_PLUGINS})
option (ENABLE_CPULOAD "Build CpuLoad for, well, monitoring the CPU load" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_DEVMON "Build Devmon, the devices monitor plugin" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
option (ENABLE_DLNIWE "Build DLNiwe, the DLNA server plugin" OFF)
option (ENABLE_DOLOZHEE "Build Dolozhee, the feature request and bug reporter" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_ELEEMINATOR "Build Leeminal, embedded LeechCraft terminal emulator" ${WITH_PLUGINS} "NOT WIN32" OFF)
cmake_dependent_option (ENABLE_FENET "Build Fenet, WM control plugin" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
option (ENABLE_FONTIAC "Build Fontiac for configurable font substitutions" ${WITH_PLUGINS})
option (ENABLE_GACTS "Build Global Actions provider plugin" ${WITH_PLUGINS})
option (ENABLE_HARBINGER "Build Harbinger, the collections manager" FALSE)
cmake_dependent_option (ENABLE_HOTSENSORS "Build HotSensors, temperature sensors quark" ${WITH_PLUGINS} "NOT WIN32" OFF)
option (ENABLE_HOTSTREAMS "Build HotStreams, radio streams provider module" ${WITH_PLUGINS})
option (ENABLE_IMGASTE "Build Imgaste, simple image paster plugin" ${WITH_PLUGINS})
option (ENABLE_INTERMUTKO "Build Intermutko for configuring HTTP Accept-Language header" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_KBSWITCH "Build KBSwitch, a keyboard plugin" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
cmake_dependent_option (ENABLE_KRIGSTASK "Build Krigstask, applications switcher" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
option (ENABLE_LACKMAN "Build LackMan, the package manager for LeechCraft" ${WITH_PLUGINS})
option (ENABLE_LADS "Build Lads, Ubuntu Unity integration layer" OFF)
option (ENABLE_LASTFMSCROBBLE "Build Last.fm client plugin" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_LAUGHTY "Build Desktop Notifications server" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
option (ENABLE_LEMON "Build LeechCraft Network Monitor" ${WITH_PLUGINS})
option (ENABLE_LHTR "Build LeechCraft HTML Text editoR" ${WITH_PLUGINS})
option (ENABLE_LIZNOO "Build Liznoo, our nice power manager" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_MELLONETRAY "Build Mellonetray for providing tray area for third-party apps" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
option (ENABLE_MONOCLE "Build Monocle, modular document viewer for LeechCraft" ${WITH_PLUGINS})
option (ENABLE_MUSICZOMBIE "Build MusicZombie, MusicBrainz.org client" ${WITH_PLUGINS})
option (ENABLE_NACHEKU "Build Nacheku, clipboard/download dir watcher" ${WITH_PLUGINS})
option (ENABLE_NAMAUTH "Build NamAuth, providing standard HTTP authentication facilities" ${WITH_PLUGINS})
option (ENABLE_NETSTOREMANAGER "Build NetStoreManager for managing network data storages like Yandex.Disk" ${WITH_PLUGINS})
option (ENABLE_OTLOZHU "Build Otlozhu, a GTD-inspired ToDo manager" ${WITH_PLUGINS})
option (ENABLE_OORONEE "Build Ooronee, a quark for handling text and images dropped on it" ${WITH_PLUGINS})
option (ENABLE_POLEEMERY "Build Poleemery, the personal finances manager" ${WITH_PLUGINS})
option (ENABLE_ROSENTHAL "Build Rosenthal, the Hunspell-based spell checker service plugin" ${WITH_PLUGINS})
option (ENABLE_SB2 "Build SB2, next-gen fluid sidebar" ${WITH_PLUGINS})
option (ENABLE_SCROBLIBRE "Build ScrobLibre, multiaccount Scrobbler API 1.2 scrobbler" ${WITH_PLUGINS})
cmake_dependent_option (ENABLE_SYSNOTIFY "Build Sysnotify for using D-Bus notification API" ${WITH_PLUGINS} "NOT WIN32 AND NOT APPLE" OFF)
option (ENABLE_TEXTOGROOSE "Build Text-o-Groose, script-based lyrics fetcher" ${WITH_PLUGINS})
option (ENABLE_TOUCHSTREAMS "Build TouchStreams, VK.com music streaming plugin" ${WITH_PLUGINS})
option (ENABLE_TPI "Build Task Progress Indicator quark plugin" ${WITH_PLUGINS})
option (ENABLE_VTYULC "Build VtyuLC, the videoplayer plugin" OFF)
option (ENABLE_VROOBY "Build Vrooby, removable storage devices manager" ${WITH_PLUGINS})
option (ENABLE_WKPLUGINS "Build WKPlugins for WebKit spellcheck and notifications" OFF)
option (ENABLE_XPROXY "Build XProxy, the advanced proxy manager" ${WITH_PLUGINS})
option (ENABLE_XTAZY "Build Xtazy, plugin for keeping track of a currently playing song" ${WITH_PLUGINS})
option (ENABLE_ZALIL "Build Zalil for uploading files to accountless filebin services" ${WITH_PLUGINS})

option (ENABLE_BLACKDASH "Build BlackDash, the dashboard plugin for LeechCraft" OFF)
option (ENABLE_CHOROID "Build Choroid, image viewer" OFF)
option (ENABLE_SNAILS "Build Snails, LeechCraft mail client" OFF)
option (ENABLE_TWIFEE "Build Twifee, the Twitter client" OFF)
option (ENABLE_CEMENTO "Build Cemento, LeechCraft common persistent storage" OFF)

# Dumb alternatives
option (ENABLE_DUMBEEP "Build Dumbeep, a dumb sound notifier for those fearing true LMP power" OFF)

cmake_dependent_option (ENABLE_Y7 "Build Y7, Windows 7 integration plugin" ${WITH_PLUGINS} "WIN32" OFF)

cmake_dependent_option (ENABLE_LAUNCHY "Build Launchy, third-party application launcher module" ${WITH_PLUGINS} "UNIX" OFF)

cmake_dependent_option (ENABLE_PIERRE "Build Pierre, Mac OS X integration layer" ${WITH_PLUGINS} "APPLE" OFF)

# Defunct
option (ENABLE_OTZERKALU "Build Otzerkalu, which allows one to download web sites recursively" OFF)

find_package(Qt5LinguistTools REQUIRED)
# Some Qt5 versions do not define a target for lconvert, so we need to find it ourselves
if (Qt5LinguistTools_FOUND)
	if (NOT TARGET Qt5::lconvert AND TARGET Qt5::lrelease)
		get_target_property(_lrelease_location Qt5::lrelease LOCATION)
		get_filename_component(_lrelease_path ${_lrelease_location} PATH)
		find_program(QT_LCONVERT_EXECUTABLE NAMES lconvert-qt5 lconvert PATHS ${_lrelease_path} NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
	elseif(TARGET Qt5::lconvert AND NOT Qt5_LCONVERT_EXECUTABLE)
		# Newer Qt5 versions define the target, but not the Qt5_LCONVERT_EXECUTABLE variable for some reason
		get_target_property(QT_LCONVERT_EXECUTABLE Qt5::lconvert LOCATION)
	endif()

	# Compatibility with the Qt4 variables
	set(QT_LRELEASE_EXECUTABLE ${Qt5_LRELEASE_EXECUTABLE})
	set(QT_LUPDATE_EXECUTABLE ${Qt5_LUPDATE_EXECUTABLE})
	if (Qt5_LCONVERT_EXECUTABLE)
		set(QT_LCONVERT_EXECUTABLE ${Qt5_LCONVERT_EXECUTABLE})
	endif()
endif()

set (GENCPP_XML_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../tools/scripts/translationstuff/gencpp.sh")

function (CreateTrsUpTargetImpl _basename Langs Sources Xml)
	string (LENGTH "${Xml}" _xmlLength)
	if (_xmlLength)
		set (Sources "${Sources};dummy.cpp")
	endif ()

	foreach (_LANG ${Langs})
		add_custom_target (${_basename}_ts_${_LANG}
				${QT_LUPDATE_EXECUTABLE} "-noobsolete" ${Sources} "-ts" "${_basename}_${_LANG}.ts"
				COMMAND "rm" "-f" "dummy.cpp"
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
	endforeach ()
	add_custom_target (${_basename}_ts
			${QT_LUPDATE_EXECUTABLE} "-noobsolete" ${Sources} "-ts" "${_basename}_\${TSLANG}.ts"
			COMMAND "rm" "-f" "dummy.cpp"
			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

	if (_xmlLength)
		add_custom_target (${_basename}_ts_convert_settings
				${GENCPP_XML_PATH} ${Xml}
				WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)
		foreach (_LANG ${Langs})
			add_dependencies (${_basename}_ts_${_LANG} ${_basename}_ts_convert_settings)
		endforeach ()
		add_dependencies (${_basename}_ts ${_basename}_ts_convert_settings)
	endif ()

	if (ARGN)
		list (LENGTH ARGN _argnLength)
		set (_argnIdx 0)

		set (_isDepends FALSE)
		foreach (_argn ${ARGN})
			if (_isDepends)
				foreach (_LANG ${Langs})
					add_dependencies (${_basename}_ts_${_LANG} ${_argn})
				endforeach ()
				add_dependencies (${_basename}_ts ${_argn})
			else ()
				string (COMPARE EQUAL ${_argn} "DEPENDS" _isDepends)
			endif ()
		endforeach ()
	endif ()
endfunction ()

function (CreateTrsUpTarget PlugName Langs Sources Forms Xml)
	string (LENGTH "${PlugName}" _pnameLength)
	set (_basename "leechcraft")
	if (_pnameLength)
		set (_basename "${_basename}_${PlugName}")
	endif ()

	list (APPEND Sources ${Forms})

	CreateTrsUpTargetImpl (${_basename} "${Langs}" "${Sources}" "${Xml}")
endfunction ()

function (CreateQmlTrsUpTarget PlugName Langs QmlFiles Xml)
	set (_basename "leechcraft_${PlugName}_qml")
	CreateTrsUpTargetImpl (${_basename} "${Langs}" "${QmlFiles}" "${Xml}")
endfunction ()

function (CreateTrs PlugName Langs CompiledTranVar)
	string (LENGTH "${PlugName}" _pnameLength)
	set (_basename "leechcraft")
	if (_pnameLength)
		set (_basename "${_basename}_${PlugName}")
	endif ()
	foreach (_LANG ${Langs})
		set (_tran ${_tran} ${_basename}_${_LANG}.ts)
		set (_ctran ${_ctran} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}_${_LANG}.qm)
	endforeach ()
	set (${CompiledTranVar} ${_ctran} PARENT_SCOPE)

	add_custom_command (OUTPUT ${_ctran}
		COMMAND "${QT_LRELEASE_EXECUTABLE}" ${_tran}
		DEPENDS ${_tran}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	install (FILES ${_ctran} DESTINATION ${LC_TRANSLATIONS_DEST})
	set (${CompiledTranVar} ${_ctran} PARENT_SCOPE)
endfunction ()

function (CreateTrsQml PlugName Langs Target QmlDirs)
	set (_basename "leechcraft_${PlugName}_qml")
	foreach (_LANG ${Langs})
		set (_tran ${_tran} ${_basename}_${_LANG}.ts)
		set (_ctran ${_ctran} ${CMAKE_CURRENT_SOURCE_DIR}/${_basename}_${_LANG}.qm)
	endforeach ()
	set (${CompiledTranVar} ${_ctran} PARENT_SCOPE)

	set (_marker "${_basename}_trsqml_marker")

	add_custom_target (${_marker}
		COMMAND "${QT_LRELEASE_EXECUTABLE}" ${_tran}
		COMMAND "${CMAKE_COMMAND}" -E tar cfj temp.tar ${_ctran}
		DEPENDS ${_tran}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	foreach (_DIR ${QmlDirs})
		string (REPLACE "/" "_" _ODIR ${_DIR})
		add_custom_target ("${_marker}_${_ODIR}_mkdir"
			COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${_DIR}/ts/"
			DEPENDS ${_marker}
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			)
		add_custom_target ("${_marker}_${_ODIR}"
			COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/temp.tar" "temp.tar"
			COMMAND "${CMAKE_COMMAND}" -E tar xfj temp.tar
			COMMAND "${CMAKE_COMMAND}" -E remove temp.tar
			DEPENDS "${_marker}_${_ODIR}_mkdir"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${_DIR}/ts/
			)
	endforeach ()

	add_custom_target ("${_marker}_whole"
		COMMAND "${CMAKE_COMMAND}" -E remove temp.tar
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		)
	foreach (_DIR ${QmlDirs})
		string (REPLACE "/" "_" _ODIR ${_DIR})
		add_dependencies ("${_marker}_whole" "${_marker}_${_ODIR}")
	endforeach ()

	add_dependencies (${Target} "${_marker}_whole")
endfunction ()

if (ENABLE_SUMMARY)
	add_subdirectory (plugins/summary)
endif ()

if (ENABLE_TORRENT)
	add_subdirectory (plugins/bittorrent)
endif ()

if (ENABLE_AGGREGATOR)
	add_subdirectory (plugins/aggregator)
endif ()

if (ENABLE_CHOROID)
	add_subdirectory (plugins/choroid)
endif ()

if (ENABLE_POSHUKU)
	add_subdirectory (plugins/poshuku)
endif ()

if (ENABLE_HTTP)
	add_subdirectory (plugins/cstp)
endif ()

if (ENABLE_HTTHARE)
	add_subdirectory (plugins/htthare)
endif ()

if (ENABLE_LMP)
	add_subdirectory (plugins/lmp)
endif ()

if (ENABLE_DBUSMANAGER)
	add_subdirectory (plugins/dbusmanager)
endif ()

if (ENABLE_DEADLYRICS)
	add_subdirectory (plugins/deadlyrics)
endif ()

if (ENABLE_SEEKTHRU)
	add_subdirectory (plugins/seekthru)
endif ()

if (ENABLE_NETWORKMONITOR)
	add_subdirectory (plugins/networkmonitor)
endif ()

if (ENABLE_HISTORYHOLDER)
	add_subdirectory (plugins/historyholder)
endif ()

if (ENABLE_NEWLIFE)
	add_subdirectory (plugins/newlife)
endif ()

if (ENABLE_ANHERO)
	add_subdirectory (plugins/anhero)
endif ()

if (ENABLE_VGRABBER)
	add_subdirectory (plugins/vgrabber)
endif ()

if (ENABLE_KINOTIFY)
	add_subdirectory (plugins/kinotify)
endif ()

if (ENABLE_AUSCRIE)
	add_subdirectory (plugins/auscrie)
endif ()

if (ENABLE_AZOTH)
	add_subdirectory (plugins/azoth)
endif ()

if (ENABLE_SECMAN)
	add_subdirectory (plugins/secman)
endif ()

if (ENABLE_QROSP)
	add_subdirectory (plugins/qrosp)
endif ()

if (ENABLE_POPISHU)
	add_subdirectory (plugins/popishu)
endif ()

if (ENABLE_HARBINGER)
	add_subdirectory (plugins/harbinger)
endif ()

if (ENABLE_HOTSENSORS)
	add_subdirectory (plugins/hotsensors)
endif ()

if (ENABLE_HOTSTREAMS)
	add_subdirectory (plugins/hotstreams)
endif ()

if (ENABLE_KRIGSTASK)
	add_subdirectory (plugins/krigstask)
endif ()

if (ENABLE_LACKMAN)
	add_subdirectory (plugins/lackman)
endif ()

if (ENABLE_LAUNCHY)
	add_subdirectory (plugins/launchy)
endif ()

if (ENABLE_SYNCER)
	add_subdirectory (plugins/syncer)
endif ()

if (ENABLE_SHELLOPEN)
	add_subdirectory (plugins/shellopen)
endif ()

if (ENABLE_GMAILNOTIFIER)
	add_subdirectory (plugins/gmailnotifier)
endif ()

if (ENABLE_OTZERKALU)
	add_subdirectory (plugins/otzerkalu)
endif ()

if (ENABLE_ADVANCEDNOTIFICATIONS)
	add_subdirectory (plugins/advancednotifications)
endif ()

if (ENABLE_GLANCE)
	add_subdirectory (plugins/glance)
endif ()

if (ENABLE_TABSLIST)
	add_subdirectory (plugins/tabslist)
endif ()

if (ENABLE_BLACKDASH)
	add_subdirectory (plugins/blackdash)
endif ()

if (ENABLE_KNOWHOW)
	add_subdirectory (plugins/knowhow)
endif ()

if (ENABLE_POLEEMERY)
	add_subdirectory (plugins/poleemery)
endif ()

if (ENABLE_ROSENTHAL)
	add_subdirectory (plugins/rosenthal)
endif ()

if (ENABLE_SB2)
	add_subdirectory (plugins/sb2)
endif ()

if (ENABLE_SCROBLIBRE)
	add_subdirectory (plugins/scroblibre)
endif ()

if (ENABLE_SNAILS)
	add_subdirectory (plugins/snails)
endif ()

if (ENABLE_SYSNOTIFY)
	add_subdirectory (plugins/sysnotify)
endif ()

if (ENABLE_TEXTOGROOSE)
	add_subdirectory (plugins/textogroose)
endif ()

if (ENABLE_TOUCHSTREAMS)
	add_subdirectory (plugins/touchstreams)
endif ()

if (ENABLE_TPI)
	add_subdirectory (plugins/tpi)
endif ()

if (ENABLE_TWIFEE)
	add_subdirectory (plugins/twifee)
endif ()

if (ENABLE_LIZNOO)
	add_subdirectory (plugins/liznoo)
endif ()

if (ENABLE_MUSICZOMBIE)
	add_subdirectory (plugins/musiczombie)
endif ()

if (ENABLE_NETSTOREMANAGER)
	add_subdirectory (plugins/netstoremanager)
endif ()

if (ENABLE_TABSESSMANAGER)
	add_subdirectory (plugins/tabsessmanager)
endif ()

if (ENABLE_LASTFMSCROBBLE)
	add_subdirectory (plugins/lastfmscrobble)
endif ()

if (ENABLE_LAUGHTY)
	add_subdirectory (plugins/laughty)
endif ()

if (ENABLE_LEMON)
	add_subdirectory (plugins/lemon)
endif ()

if (ENABLE_LHTR)
	add_subdirectory (plugins/lhtr)
endif ()

if (ENABLE_CEMENTO)
	add_subdirectory (plugins/cemento)
endif ()

if (ENABLE_CPULOAD)
	add_subdirectory (plugins/cpuload)
endif ()

if (ENABLE_CERTMGR)
	add_subdirectory (plugins/certmgr)
endif ()

if (ENABLE_DEVMON)
	add_subdirectory (plugins/devmon)
endif ()

if (ENABLE_DLNIWE)
	add_subdirectory (plugins/dlniwe)
endif ()

if (ENABLE_ELEEMINATOR)
	add_subdirectory (plugins/eleeminator)
endif ()

if (ENABLE_FENET)
	add_subdirectory (plugins/fenet)
endif ()

if (ENABLE_FONTIAC)
	add_subdirectory (plugins/fontiac)
endif ()

if (ENABLE_GACTS)
	add_subdirectory (plugins/gacts)
endif ()

if (ENABLE_BLASQ)
	add_subdirectory (plugins/blasq)
endif ()

if (ENABLE_BLOGIQUE)
	add_subdirectory (plugins/blogique)
endif ()

if (ENABLE_MELLONETRAY)
	add_subdirectory (plugins/mellonetray)
endif ()

if (ENABLE_MONOCLE)
	add_subdirectory (plugins/monocle)
endif ()

if (ENABLE_PINTAB)
	add_subdirectory (plugins/pintab)
endif ()

if (ENABLE_POGOOGLUE)
	add_subdirectory (plugins/pogooglue)
endif ()

if (ENABLE_OTLOZHU)
	add_subdirectory (plugins/otlozhu)
endif ()

if (ENABLE_OORONEE)
	add_subdirectory (plugins/ooronee)
endif ()

if (ENABLE_IMGASTE)
	add_subdirectory (plugins/imgaste)
endif ()

if (ENABLE_INTERMUTKO)
	add_subdirectory (plugins/intermutko)
endif ()

if (ENABLE_KBSWITCH)
	add_subdirectory (plugins/kbswitch)
endif ()

if (ENABLE_XPROXY)
	add_subdirectory (plugins/xproxy)
endif ()

if (ENABLE_DOLOZHEE)
	add_subdirectory (plugins/dolozhee)
endif ()

if (ENABLE_NACHEKU)
	add_subdirectory (plugins/nacheku)
endif ()

if (ENABLE_NAMAUTH)
	add_subdirectory (plugins/namauth)
endif ()

if (ENABLE_VROOBY)
	add_subdirectory (plugins/vrooby)
endif ()

if (ENABLE_XTAZY)
	add_subdirectory (plugins/xtazy)
endif ()

if (ENABLE_ZALIL)
	add_subdirectory (plugins/zalil)
endif ()

if (ENABLE_DUMBEEP)
	add_subdirectory (plugins/dumbeep)
endif ()

if (ENABLE_VTYULC)
	add_subdirectory (plugins/vtyulc)
endif ()

if (ENABLE_WKPLUGINS)
	add_subdirectory (plugins/wkplugins)
endif ()

if (UNIX AND NOT APPLE)
	if (ENABLE_LADS)
		add_subdirectory (plugins/lads)
	endif ()
endif ()

if (WIN32)
	if (ENABLE_Y7)
		add_subdirectory (plugins/y7)
	endif ()
endif ()

if (APPLE)
	if (ENABLE_PIERRE)
		add_subdirectory (plugins/pierre)
	endif ()
endif ()

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

option (BUILD_CODEGEN "Build LeechCraft bindings generator (development only)" OFF)
if (BUILD_CODEGEN)
	add_subdirectory (codegen)
endif ()

if (UNIX)
	add_custom_target (data_leechcraft_gz ALL
		"${CMAKE_CURRENT_SOURCE_DIR}/data/compress"
		DEPENDS data/leechcraft-qt5.1
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/data
		)
endif ()

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
	install (DIRECTORY data/ DESTINATION share/man/man1 FILES_MATCHING PATTERN "*.gz")
	install (FILES freedesktop/leechcraft-qt5.desktop DESTINATION share/applications)
	install (FILES scripts/leechcraft-add-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
	install (FILES scripts/leechcraft-handle-file DESTINATION bin PERMISSIONS
		OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
		WORLD_EXECUTE WORLD_READ)
endif ()
