# cOPyright 1999-2008 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit cmake-utils qt4 flag-o-matic subversion
 
DESCRIPTION="LeechCraft - extensible cross-platform download manager"
HOMEPAGE="http://deviant-soft.ws/"
ESVN_REPO_URI="svn://deviant-soft.ws/leechcraft/trunk/src"
 
LICENSE="QPL"
SLOT="0"
KEYWORDS=""
IUSE=""
 
DEPEND="$(qt4_min_version 4.4)
	>=net-libs/rb_libtorrent-0.15_rc1"
RDEPEND=""
 
S=${WORKDIR}/
 
pkg_setup ()
{
	QT4_BUILD_WITH_USE_CHECK="jpeg png"
	qt4_pkg_setup
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
