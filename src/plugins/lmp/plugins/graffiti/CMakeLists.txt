cmake_minimum_required (VERSION 3.10)
project (lmp_graffiti)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		graffiti.cpp
		graffititab.cpp
		filesmodel.cpp
		renamedialog.cpp
		fileswatcher.cpp
		cuesplitter.cpp
		progressmanager.cpp
		tagsfetchmanager.cpp
		reciterator.cpp
		literals.cpp
	QT_COMPONENTS Concurrent Widgets
	LINK_LIBRARIES PkgConfig::TagLib
	INSTALL_SHARE
	)
