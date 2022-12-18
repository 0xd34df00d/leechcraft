cmake_minimum_required (VERSION 3.10)
project (pierre)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		pierre.cpp
		fullscreen.mm
		reopenhandler.mm
	QT_COMPONENTS Core
	LINK_LIBRARIES "-framework AppKit"
	)

if (NOT USE_UNIX_LAYOUT)
	install (DIRECTORY data/ DESTINATION ${LC_BINDIR}/..)
endif ()
