# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

EAPI="2"

EGIT_REPO_URI="git://github.com/0xd34df00d/leechcraft.git"
inherit git cmake-utils

DESCRIPTION="Opensource network client providing a full-featured web browser, BitTorrent client and much more."
HOMEPAGE="http://leechcraft.org/"

LICENSE="GPL-3"
SLOT="0"
KEYWORDS=""
IUSE="+poshuku +torrent +aggregator +dbusmanager deadlyrics historyholder
	lmp networkmonitor +seekthru debug"

DEPEND="dev-libs/boost:1.37
		>=x11-libs/qt-gui-4.5.1
		>=x11-libs/qt-core-4.5.1
		>=x11-libs/qt-sql-4.5.1
		>=x11-libs/qt-script-4.5.1
		torrent? ( =net-libs/rb_libtorrent-9999 )
		lmp? ( media-sound/phonon )
		poshuku? ( >=x11-libs/qt-webkit-4.5.1 )"
RDEPEND="${DEPEND}"

src_unpack()
{
	git_src_unpack
}

src_configure()
{
	
	if use debug ; then
		CMAKE_BUILD_TYPE="RelWithDebInfo"
	else
		CMAKE_BUILD_TYPE="Release"
	fi

	mycmakeargs="${mycmakeargs}
				-DENABLE_CONFIGURABLE=ON
				-DENABLE_HTTP=ON
				$(cmake-utils_use_enable poshuku POSHUKU)
				$(cmake-utils_use_enable torrent TORRENT)
				$(cmake-utils_use_enable aggregator AGGREGATOR)
				$(cmake-utils_use_enable dbusmanager DBUSMANAGER)
				$(cmake-utils_use_enable deadlyrics DEADLIRYCS)
				$(cmake-utils_use_enable historyholder HISTORYHOLDER)
				$(cmake-utils_use_enable lmp LMP)
				$(cmake-utils_use_enable networkmonitor NETWORKMONITOR)
				$(cmake-utils_use_enable seekthru SEEKTHRU)"
	S="${WORKDIR}/${P}/src"
	cmake-utils_src_configure
}

src_install()
{
	cmake-utils_src_install
	doicon resources/leechcraft.png
	make_desktop_entry leechcraft "LeechCraft" leechcraft.png
}
