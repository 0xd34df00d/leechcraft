cmake_minimum_required (VERSION 3.10)
project (leechcraft_secman)
include (InitLCPlugin NO_POLICY_SCOPE)

option (WITH_SECMAN_EXPOSE_CONTENTSDISPLAY "Build the GUI for viewing the data stored in SecMan" OFF)
if (WITH_SECMAN_EXPOSE_CONTENTSDISPLAY)
	add_definitions (-DSECMAN_EXPOSE_CONTENTSDISPLAY)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		secman.cpp
		core.cpp
		contentsdisplaydialog.cpp
		persistentstorage.cpp
	QT_COMPONENTS Widgets
	INSTALL_SHARE
	)

option (TESTS_SECMAN "Enable SecMan tests")
SUBPLUGIN (SECURESTORAGE "Enable SecMan SecureStorage, the advanced encrypted private data storage" OFF)
SUBPLUGIN (SIMPLESTORAGE "Enable SecMan SimpleStorage, the basic unencrypted private data storage" ON)
