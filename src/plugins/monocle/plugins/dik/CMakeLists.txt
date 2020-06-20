cmake_minimum_required (VERSION 3.3)
project (leechcraft_monocle_dik)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		dik.cpp
		mobiparser.cpp
		decompressor.cpp
		util.cpp
		document.cpp
	QT_COMPONENTS Gui Xml
	)
