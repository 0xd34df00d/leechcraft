cmake_minimum_required (VERSION 3.10)
project (azoth_modnok)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		modnok.cpp
	SETTINGS azothmodnoksettings.xml
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)
install (PROGRAMS lc_azoth_modnok_latexconvert.sh DESTINATION ${LC_SHARE_DEST}/azoth)
