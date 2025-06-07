cmake_minimum_required (VERSION 3.10)
project (lmp)
include (InitLCPlugin NO_POLICY_SCOPE)
include (CMakeDependentOption)

find_package (PkgConfig)
pkg_check_modules (TagLib REQUIRED IMPORTED_TARGET taglib)
pkg_check_modules (GStreamer REQUIRED IMPORTED_TARGET gstreamer-1.0)

cmake_dependent_option (ENABLE_LMP_MPRIS "Enable MPRIS support for LMP" ON "NOT WIN32" OFF)

if (ENABLE_LMP_MPRIS)
	set (EXTRA_DBUS_LIB DBus)
endif ()

add_compile_definitions ($<$<BOOL:{ENABLE_LMP_MPRIS}>:ENABLE_MPRIS>)

add_util_library (lmp-util
	SRCS
		util/lmp/gstutil.cpp
		util/lmp/filtersettingsmanager.cpp
		util/lmp/mediainfo.cpp
		util/lmp/util.cpp
	USES Widgets
	DEPENDS PkgConfig::GStreamer ${LEECHCRAFT_LIBRARIES}
	)

LC_DEFINE_PLUGIN (
	SRCS
		lmp.cpp
		playertab.cpp
		player.cpp
		core.cpp
		localfileresolver.cpp
		playlistdelegate.cpp
		localcollection.cpp
		localcollectionstorage.cpp
		util.cpp
		collectiontypes.cpp
		collectiondelegate.cpp
		nowplayingwidget.cpp
		staticplaylistmanager.cpp
		playlistmanager.cpp
		similarview.cpp
		albumartmanager.cpp
		lmpsystemtrayicon.cpp
		fsbrowserwidget.cpp
		fsmodel.cpp
		rootpathsettingsmanager.cpp
		localcollectionwatcher.cpp
		recommendationswidget.cpp
		radiowidget.cpp
		releaseswidget.cpp
		playlistundocommand.cpp
		biowidget.cpp
		biopropproxy.cpp
		audiopropswidget.cpp
		playlistwidget.cpp
		aalabeleventfilter.cpp
		collectionsortermodel.cpp
		collectionstatsdialog.cpp
		eventswidget.cpp
		plmanagerwidget.cpp
		nowplayingpixmaphandler.cpp
		lmpproxy.cpp
		sortingcriteria.cpp
		sortingcriteriadialog.cpp
		similarmodel.cpp
		hypeswidget.cpp
		bioviewmanager.cpp
		similarviewmanager.cpp
		artistbrowsertab.cpp
		albumartmanagerdialog.cpp
		progressmanager.cpp
		volumeslider.cpp
		seekslider.cpp
		palettefixerfilter.cpp
		playlistmodel.cpp
		volumenotifycontroller.cpp
		radiomanager.cpp
		radiocustomstreams.cpp
		radiocustomstation.cpp
		radiocustomdialog.cpp
		radiopilesmanager.cpp
		stdartistactionsmanager.cpp
		notificationplayer.cpp
		effectsmanager.cpp
		recursivedirwatcher.cpp
		rganalysismanager.cpp
		rgfiltercontroller.cpp
		collectionsmanager.cpp
		collectionwidget.cpp
		localcollectionmodel.cpp
		playerrulesmanager.cpp
		hookinterconnector.cpp
		diaginfocollector.cpp
		npstateupdater.cpp
		nptooltiphook.cpp
		previewcharacteristicinfo.cpp
		radiotracksgrabdialog.cpp
		nativeplaylist.cpp
		playlistwidgetviewexpander.cpp
		sourceerrorhandler.cpp
		literals.cpp
		collectionnormalizer.cpp
		engine/audiosource.cpp
		engine/output.cpp
		engine/path.cpp
		engine/pathelement.cpp
		engine/rganalyser.cpp
		engine/rgfilter.cpp
		engine/sourceobject.cpp
		sync/cloudwidget.cpp
		sync/clouduploadmanager.cpp
		sync/clouduploader.cpp
		sync/copymanager.cpp
		sync/devicesbrowserwidget.cpp
		sync/formats.cpp
		sync/syncmanagerbase.cpp
		sync/syncmanager.cpp
		sync/syncunmountablemanager.cpp
		sync/transcodejob.cpp
		sync/transcodemanager.cpp
		sync/transcodingparams.cpp
		sync/transcodingparamswidget.cpp
		sync/unmountabledevmanager.cpp
		sync/uploadmodel.cpp
		playlistparsers/commonpl.cpp
		playlistparsers/playlist.cpp
		playlistparsers/playlistfactory.cpp
		playlistparsers/m3u.cpp
		playlistparsers/pls.cpp
		playlistparsers/xspf.cpp
		$<IF:$<PLATFORM_ID:Darwin>,
			recursivedirwatcher_mac.mm,
			recursivedirwatcher_generic.cpp>
		$<$<BOOL:${ENABLE_LMP_MPRIS}>:
			mpris/instance.cpp
			mpris/mediaplayer2adaptor.cpp
			mpris/playeradaptor.cpp
			mpris/fdopropsadaptor.cpp
			>
	RESOURCES resources.qrc
	SETTINGS lmpsettings.xml lmpfilterrgsettings.xml
	QT_COMPONENTS Concurrent Network QuickWidgets Sql Xml ${EXTRA_DBUS_LIB}
	LINK_LIBRARIES
		PkgConfig::TagLib
		PkgConfig::GStreamer
		$<$<PLATFORM_ID:Darwin>:"-framework Foundation -framework CoreServices">
		leechcraft-lmp-util
	INSTALL_SHARE
	INSTALL_DESKTOP
	)

SUBPLUGIN (BRAINSLUGZ "Enable BrainSlugz, plugin for checking collection completeness" ON)
SUBPLUGIN (DUMBSYNC "Enable DumbSync, plugin for syncing with Flash-like media players" ON)
SUBPLUGIN (FRADJ "Enable Fradj for multiband configurable equalizer" ON)
SUBPLUGIN (GRAFFITI "Enable Graffiti for manipulating audio file tags" ON)
SUBPLUGIN (HTTSTREAM "Enable HttStream for streaming music from LMP via HTTP" ON)
SUBPLUGIN (JOS "Enable jOS for synchronizing with iOS devices" OFF)
SUBPLUGIN (MP3TUNES "Enable plugin for syncing and using the mp3tunes.com service" ON)
SUBPLUGIN (MTPSYNC "Enable plugin for syncing with MTP devices" OFF)
SUBPLUGIN (POTORCHU "Enable visualization effects plugin" OFF)
SUBPLUGIN (PPL "Enable Portable Player Logging support" ON)
