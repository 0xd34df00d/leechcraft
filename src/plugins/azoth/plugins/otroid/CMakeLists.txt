cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_otroid)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (LibOTR REQUIRED)
message (STATUS "Found OTR: ${LIBOTR_LIBRARY}")
include_directories (${LIBOTR_INCLUDE_DIR})

LC_DEFINE_PLUGIN (azoth_otroid
	SRCS
		otroid.cpp
		otrhandler.cpp
		authenticator.cpp
		initiateauthdialog.cpp
		fpmanager.cpp
		privkeymanager.cpp
		xmlsettingsmanager.cpp
		util.cpp
	FORMS initiateauthdialog.ui
	SETTINGS azothotroidsettings.xml
	QT_COMPONENTS Concurrent Widgets
	LINK_LIBRARIES ${LIBOTR_LIBRARIES} leechcraft_azoth_common
	INSTALL_SHARE
	)
