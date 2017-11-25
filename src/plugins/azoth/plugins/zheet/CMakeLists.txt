cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_zheet)
include (InitLCPlugin NO_POLICY_SCOPE)

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (Libmsn REQUIRED)
find_package (OpenSSL REQUIRED)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LIBMSN_INCLUDE_DIR}
	${OPENSSL_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (ZHEET_SRCS
	zheet.cpp
	core.cpp
	msnprotocol.cpp
	msnaccount.cpp
	callbacks.cpp
	zheetutil.cpp
	msnaccountconfigwidget.cpp
	msnbuddyentry.cpp
	msnmessage.cpp
	sbmanager.cpp
	groupmanager.cpp
	manageblacklistdialog.cpp
	transfermanager.cpp
	transferjob.cpp
	)
set (ZHEET_FORMS
	msnaccountconfigwidget.ui
	manageblacklistdialog.ui
	)
set (ZHEET_RESOURCES azothzheetresources.qrc)
CreateTrs ("azoth_zheet" "en;es;ru_RU;uk_UA" ZHEET_COMPILED_TRANSLATIONS)

QtWrapUi (ZHEET_UIS_H ${ZHEET_FORMS})
QtAddResources (ZHEET_RCCS ${ZHEET_RESOURCES})

add_library (leechcraft_azoth_zheet SHARED
	${ZHEET_COMPILED_TRANSLATIONS}
	${ZHEET_SRCS}
	${ZHEET_UIS_H}
	${ZHEET_RCCS}
	)
target_link_libraries (leechcraft_azoth_zheet
	${LEECHCRAFT_LIBRARIES}
	${LIBMSN_LIBRARIES}
	${OPENSSL_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_zheet DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ZHEET_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
#install (FILES azothzheetsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_zheet Network Widgets Xml)
