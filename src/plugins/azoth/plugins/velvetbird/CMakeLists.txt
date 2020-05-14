cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_velvetbird)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (GLIB2 REQUIRED)
find_package (LibPurple REQUIRED)

include_directories (SYSTEM
	${GLIB2_INCLUDE_DIR}
	${LIBPURPLE_INCLUDE_DIRS}
	)

add_definitions (${LIBPURPLE_DEFINITIONS})

LC_DEFINE_PLUGIN (azoth_velvetbird
	SRCS
		velvetbird.cpp
		protomanager.cpp
		protocol.cpp
		account.cpp
		accregfirstpage.cpp
		util.cpp
		buddy.cpp
		convimmessage.cpp
		accountconfigdialog.cpp
	FORMS
		accregfirstpage.ui
		accountconfigdialog.ui
	SETTINGS 
	QT_COMPONENTS Widgets
	LINK_LIBRARIES ${GLIB2_LIBRARIES} ${LIBPURPLE_LIBRARIES}
	INSTALL_SHARE
	)
