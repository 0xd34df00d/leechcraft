# cOPyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit cmake-utils qt4 flag-o-matic

DESCRIPTION="LeechCraft - extensible cross-platform download manager"
HOMEPAGE="http://sourceforge.net/projects/leechcraft"
SRC_URI="http://deviant-soft.ws/leechcraft/${P}.tar.bz2"

LICENSE="QPL"
SLOT="0"
KEYWORDS="~x86 ~amd64"
IUSE=""

DEPEND="$(qt4_min_version 4.3)
		>=net-libs/rb_libtorrent-0.13_pre1912"
RDEPEND=""

pkg_setup ()
{
	QT4_BUILD_WITH_USE_CHECK="jpeg png"
	qt4_pkg_setup
}

src_unpack ()
{
	unpack ${A}
}

src_compile ()
{
	filter-flags -freorder-blocks-and-partition
	cmake-utils_src_compile
}

src_install ()
{
	cmake-utils_src_install
}

