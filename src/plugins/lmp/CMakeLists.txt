cmake_minimum_required (VERSION 2.8)
project (leechcraft_lmp)
include (InitLCPlugin OPTIONAL)
include (CMakeDependentOption)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# This stuff won't be need by GStreamer 1.0.
find_package (LibXml2 REQUIRED)
find_package (GStreamer REQUIRED)
find_package (GLIB2 REQUIRED)

find_package (Taglib REQUIRED)
add_definitions (${TAGLIB_CFLAGS})

cmake_dependent_option (ENABLE_LMP_MPRIS "Enable MPRIS support for LMP" ON "NOT WIN32" OFF)

option (ENABLE_LMP_LIBGUESS "Enable tags recoding using the LibGuess library" ON)
if (ENABLE_LMP_LIBGUESS)
	find_package (LibGuess REQUIRED)
	add_definitions (-DWITH_LIBGUESS)
endif ()

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${TAGLIB_INCLUDES}
	${GSTREAMER_INCLUDE_DIR}
	${GLIB2_INCLUDE_DIR}
	${LIBXML2_INCLUDE_DIR}
	${LibGuess_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (COMMON_SRCS
	mediainfo.cpp
	util/lmp/gstutil.cpp
	util/lmp/filtersettingsmanager.cpp
	util/lmp/util.cpp
	)
add_library (leechcraft_lmp_common STATIC
	${COMMON_SRCS}
	)
set_target_properties (leechcraft_lmp_common PROPERTIES POSITION_INDEPENDENT_CODE True)
target_link_libraries (leechcraft_lmp_common
	${LibGuess_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

FindQtLibs (leechcraft_lmp_common Core)

set (SRCS
	lmp.cpp
	xmlsettingsmanager.cpp
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
	previewhandler.cpp
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
	)
set (FORMS
	playertab.ui
	nowplayingwidget.ui
	fsbrowserwidget.ui
	recommendationswidget.ui
	radiowidget.ui
	releaseswidget.ui
	biowidget.ui
	audiopropswidget.ui
	playlistwidget.ui
	collectionstatsdialog.ui
	eventswidget.ui
	plmanagerwidget.ui
	sortingcriteriadialog.ui
	hypeswidget.ui
	artistbrowsertab.ui
	albumartmanagerdialog.ui
	volumeslider.ui
	seekslider.ui
	radiocustomdialog.ui
	collectionwidget.ui
	radiotracksgrabdialog.ui
	sync/cloudwidget.ui
	sync/devicesbrowserwidget.ui
	sync/transcodingparamswidget.ui
	)
set (RESOURCES resources.qrc)
CreateTrs ("lmp" "en;es;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("lmp" "en;ru_RU" "${SRCS}" "${FORMS}" "lmpsettings.xml")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

set (ADDITIONAL_LIBRARIES)
if (NOT APPLE)
	set (SRCS ${SRCS} recursivedirwatcher_generic.cpp)
else ()
	set (ADDITIONAL_LIBRARIES "-framework Foundation;-framework CoreServices")
	set (SRCS ${SRCS} recursivedirwatcher_mac.mm)
endif ()

add_library (leechcraft_lmp SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
if (ENABLE_LMP_MPRIS)
	add_subdirectory (mpris)
	add_definitions (-DENABLE_MPRIS)
endif ()
target_link_libraries (leechcraft_lmp
	${LEECHCRAFT_LIBRARIES}
	${TAGLIB_LIBRARIES}
	${GSTREAMER_LIBRARIES}
	${GLIB2_LIBRARIES}
	${LIBXML2_LIBRARIES}
	${LibGuess_LIBRARIES}
	${ADDITIONAL_LIBRARIES}

	${MPRIS_SUBLIB}
	leechcraft_lmp_common
	)

install (TARGETS leechcraft_lmp DESTINATION ${LC_PLUGINS_DEST})
install (FILES lmpsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES lmpfilterrgsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml5/lmp DESTINATION ${LC_QML5_DEST})
install (DIRECTORY interfaces DESTINATION include/leechcraft)
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-lmp-qt5.desktop DESTINATION share/applications)
endif ()

FindQtLibs (leechcraft_lmp Concurrent Network QuickWidgets Sql Xml)
if (ENABLE_LMP_MPRIS)
	FindQtLibs (leechcraft_lmp DBus)
endif ()

option (ENABLE_LMP_BRAINSLUGZ "Enable BrainSlugz, plugin for checking collection completeness" ON)
option (ENABLE_LMP_DUMBSYNC "Enable DumbSync, plugin for syncing with Flash-like media players" ON)
option (ENABLE_LMP_FRADJ "Enable Fradj for multiband configurable equalizer" ON)
option (ENABLE_LMP_GRAFFITI "Enable Graffiti for manipulating audio file tags" ON)
option (ENABLE_LMP_HTTSTREAM "Enable HttStream for streaming music from LMP via HTTP" ON)
option (ENABLE_LMP_JOS "Enable jOS for synchronizing with iOS devices" OFF)
option (ENABLE_LMP_MP3TUNES "Enable plugin for syncing and using the mp3tunes.com service" ON)
option (ENABLE_LMP_MTPSYNC "Enable plugin for syncing with MTP devices" OFF)
option (ENABLE_LMP_POTORCHU "Enable visualization effects plugin" OFF)
option (ENABLE_LMP_PPL "Enable Portable Player Logging support" ON)

if (ENABLE_LMP_BRAINSLUGZ)
	add_subdirectory (plugins/brainslugz)
endif ()
if (ENABLE_LMP_DUMBSYNC)
	add_subdirectory (plugins/dumbsync)
endif ()
if (ENABLE_LMP_FRADJ)
	add_subdirectory (plugins/fradj)
endif ()
if (ENABLE_LMP_GRAFFITI)
	add_subdirectory (plugins/graffiti)
endif ()
if (ENABLE_LMP_HTTSTREAM)
	add_subdirectory (plugins/httstream)
endif ()
if (ENABLE_LMP_JOS)
	add_subdirectory (plugins/jos)
endif ()
if (ENABLE_LMP_MP3TUNES)
	add_subdirectory (plugins/mp3tunes)
endif ()
if (ENABLE_LMP_MTPSYNC)
	add_subdirectory (plugins/mtpsync)
endif ()
if (ENABLE_LMP_POTORCHU)
	add_subdirectory (plugins/potorchu)
endiF (ENABLE_LMP_POTORCHU)
if (ENABLE_LMP_PPL)
	add_subdirectory (plugins/ppl)
endiF (ENABLE_LMP_PPL)
