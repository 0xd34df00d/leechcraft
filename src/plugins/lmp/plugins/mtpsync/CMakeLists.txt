cmake_minimum_required (VERSION 3.10)
project (lmp_mtpsync)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (MTP REQUIRED IMPORTED_TARGET libmtp)

LC_DEFINE_PLUGIN (
	SRCS mtpsync.cpp
	QT_COMPONENTS Concurrent Gui
	LINK_LIBRARIES PkgConfig::MTP
	)
