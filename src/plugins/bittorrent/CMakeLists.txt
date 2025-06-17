cmake_minimum_required (VERSION 3.10)
project (bittorrent)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Libtorrent REQUIRED IMPORTED_TARGET libtorrent-rasterbar)

option (ENABLE_BITTORRENT_GEOIP "Enable support for GeoIP in BitTorrent" OFF)

if (ENABLE_BITTORRENT_GEOIP)
	add_definitions (-DENABLE_GEOIP)
	pkg_check_modules (MMDB REQUIRED IMPORTED_TARGET libmaxminddb)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		components/models/addtorrentfilesmodel.cpp
		components/models/peersmodel.cpp
		components/models/piecesmodel.cpp
		components/models/tabviewproxymodel.cpp
		components/models/torrentfilesmodel.cpp
		torrentplugin.cpp
		tabwidget.cpp
		core.cpp
		addtorrent.cpp
		newtorrentwizard.cpp
		pieceswidget.cpp
		filesviewdelegate.cpp
		wizardgenerator.cpp
		startupfirstpage.cpp
		livestreammanager.cpp
		livestreamdevice.cpp
		speedselectoraction.cpp
		torrentmaker.cpp
		torrenttab.cpp
		torrenttabwidget.cpp
		addmagnetdialog.cpp
		torrenttabfileswidget.cpp
		sessionsettingsmanager.cpp
		peerswidget.cpp
		ltutils.cpp
		listactions.cpp
		alertdispatcher.cpp

		banpeersdialog.cpp
		addwebseeddialog.cpp
		ipvalidators.cpp
		fastspeedcontrolwidget.cpp
		singletrackerchanger.cpp
		cachedstatuskeeper.cpp
		geoip.cpp
		movetorrentfiles.cpp
		trackerschanger.cpp
		addpeerdialog.cpp
		addmultipletorrents.cpp
		ipfilterdialog.cpp
	SETTINGS torrentsettings.xml
	QT_COMPONENTS Xml Widgets
	LINK_LIBRARIES PkgConfig::Libtorrent $<$<BOOL:${ENABLE_BITTORRENT_GEOIP}>:PkgConfig::MMDB>
	INSTALL_SHARE
	INSTALL_DESKTOP
	)
