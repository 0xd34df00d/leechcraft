cmake_minimum_required (VERSION 3.3)
project (leechcraft_bittorrent)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Libtorrent REQUIRED IMPORTED_TARGET libtorrent-rasterbar)

option (ENABLE_BITTORRENT_GEOIP "Enable support for GeoIP in BitTorrent" OFF)

set (MMDB_IMPORTED_TARGET)
if (ENABLE_BITTORRENT_GEOIP)
	add_definitions (-DENABLE_GEOIP)
	pkg_check_modules (MMDB REQUIRED IMPORTED_TARGET libmaxminddb)
	set (MMDB_IMPORTED_TARGET PkgConfig::MMDB)
endif ()

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	torrentplugin.cpp
	tabwidget.cpp
	core.cpp
	addtorrent.cpp
	newtorrentwizard.cpp
	intropage.cpp
	firststep.cpp
	secondstep.cpp
	thirdstep.cpp
	addmultipletorrents.cpp
	pieceswidget.cpp
	xmlsettingsmanager.cpp
	piecesmodel.cpp
	torrentfilesmodel.cpp
	peersmodel.cpp
	filesviewdelegate.cpp
	movetorrentfiles.cpp
	trackerschanger.cpp
	peerstablinker.cpp
	addpeerdialog.cpp
	ipvalidators.cpp
	addwebseeddialog.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	fastspeedcontrolwidget.cpp
	banpeersdialog.cpp
	ipfilterdialog.cpp
	livestreammanager.cpp
	livestreamdevice.cpp
	speedselectoraction.cpp
	torrentmaker.cpp
	singletrackerchanger.cpp
	torrenttab.cpp
	torrenttabwidget.cpp
	tabviewproxymodel.cpp
	notifymanager.cpp
	addmagnetdialog.cpp
	addtorrentfilesmodel.cpp
	torrenttabfileswidget.cpp
	sessionsettingsmanager.cpp
	cachedstatuskeeper.cpp
	geoip.cpp
	)

set (FORMS
    tabwidget.ui
	addtorrent.ui
	addmultipletorrents.ui
	newtorrentfirststep.ui
	newtorrentsecondstep.ui
	newtorrentthirdstep.ui
	movetorrentfiles.ui
	trackerschanger.ui
	addpeerdialog.ui
	addwebseeddialog.ui
	startupfirstpage.ui
	fastspeedcontrolwidget.ui
	banpeersdialog.ui
	ipfilterdialog.ui
	singletrackerchanger.ui
	torrenttab.ui
	torrenttabwidget.ui
	addmagnetdialog.ui
	torrenttabfileswidget.ui
	)

set (RESOURCES bittorrentresources.qrc)
CreateTrs ("bittorrent" "ar;be;de;en;eo;es;fr;it;oc;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("bittorrent" "en;ru_RU" "${SRCS}" "${FORMS}" "torrentsettings.xml")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_bittorrent SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)

target_link_libraries (leechcraft_bittorrent
	${LEECHCRAFT_LIBRARIES}
	PkgConfig::Libtorrent
	${MMDB_IMPORTED_TARGET}
)
install (TARGETS leechcraft_bittorrent DESTINATION ${LC_PLUGINS_DEST})
install (FILES torrentsettings.xml DESTINATION ${LC_SETTINGS_DEST})
if (UNIX AND NOT APPLE)
	install (FILES freedesktop/leechcraft-bittorrent-qt5.desktop DESTINATION share/applications)
endif ()

FindQtLibs (leechcraft_bittorrent Xml Widgets)
