# Copyright 1999-2009 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: /var/cvsroot/gentoo-x86/net-libs/rb_libtorrent/rb_libtorrent-0.14.4.ebuild,v 1.1 2009/06/06 16:30:06 armin76 Exp $

EAPI="2"

ESVN_REPO_URI="https://libtorrent.svn.sourceforge.net/svnroot/libtorrent/trunk"

inherit eutils cmake-utils subversion

MY_P=${P/rb_/}
MY_P=${MY_P/torrent/torrent-rasterbar}
S=${WORKDIR}/${MY_P}

DESCRIPTION="BitTorrent library written in C++ for *nix."
HOMEPAGE="http://www.rasterbar.com/products/libtorrent/"

LICENSE="BSD"
SLOT="0"
KEYWORDS=""

IUSE="debug examples test +encryption geoip +resolvecountries logging vlogging"

DEPEND="
	>=dev-libs/boost-1.34
	|| ( >=dev-libs/boost-1.35 dev-cpp/asio )
	sys-libs/zlib
	!net-libs/libtorrent"
RDEPEND="${DEPEND}"

src_unpack()
{
	subversion_src_unpack
}

src_configure()
{
	if use debug ; then
		CMAKE_BUILD_TYPE="RelWithDebInfo"
	else
		CMAKE_BUILD_TYPE="Release"
	fi

	mycmakeargs="${mycmakeargs}
				-Dbuild_examples=OFF
				-Dbuild_tests=OFF
				-Dresolve-countries=OFF
				-Dencryption=OFF
				$(cmake-utils_use examples build_examples)
				$(cmake-utils_use test build_tests)
				$(cmake-utils_use resolvecountries resolve-countries)
				$(cmake-utils_use encryption encryption)
				$(cmake-utils_use geoip geoip)
				$(cmake-utils_use logging logging)
				$(cmake-utils_use vlogging verbose-logging)"
	cmake-utils_src_configure
}

src_install()
{
	cmake-utils_src_install
	dodoc ChangeLog AUTHORS NEWS README
}
