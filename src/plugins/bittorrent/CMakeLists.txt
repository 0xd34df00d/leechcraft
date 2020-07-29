cmake_minimum_required (VERSION 3.10)
project (leechcraft_bittorrent)
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
	SETTINGS torrentsettings.xml
	QT_COMPONENTS Xml Widgets
	LINK_LIBRARIES PkgConfig::Libtorrent $<$<BOOL:${ENABLE_BITTORRENT_GEOIP}>:PkgConfig::MMDB>
	INSTALL_SHARE
	INSTALL_DESKTOP
	)
