cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_vader)
include (InitLCPlugin)

option (ENABLE_AZOTH_VADER_VERBOSE_LOG "Debug protocol messages & similar stuff with increased verbosity" OFF)

if (ENABLE_AZOTH_VADER_VERBOSE_LOG)
	add_definitions (-DPROTOCOL_LOGGING)
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (VADER_SRCS
	vader.cpp
	mrimprotocol.cpp
	mrimaccount.cpp
	mrimaccountconfigwidget.cpp
	mrimbuddy.cpp
	mrimmessage.cpp
	vaderutil.cpp
	groupmanager.cpp
	smsdialog.cpp
	selfavatarfetcher.cpp
	xmlsettingsmanager.cpp
	vcarddialog.cpp
	avatarstimestampstorage.cpp
	proto/headers.cpp
	proto/conversions.cpp
	proto/connection.cpp
	proto/packet.cpp
	proto/packetfactory.cpp
	proto/packetextractor.cpp
	proto/exceptions.cpp
	proto/halfpacket.cpp
	proto/balancer.cpp
	proto/typingmanager.cpp
	)
set (VADER_FORMS
	mrimaccountconfigwidget.ui
	smsdialog.ui
	vcarddialog.ui
	)
set (VADER_RESOURCES azothvaderresources.qrc)

CreateTrs ("azoth_vader" "en;es;ru_RU;uk_UA" VADER_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_vader" "en;ru_RU" "${VADER_SRCS}" "${VADER_FORMS}" "azothvadersettings.xml")

QtWrapUi (VADER_UIS_H ${VADER_FORMS})
QtAddResources (VADER_RCCS ${VADER_RESOURCES})

add_library (leechcraft_azoth_vader SHARED
	${VADER_COMPILED_TRANSLATIONS}
	${VADER_SRCS}
	${VADER_UIS_H}
	${VADER_RCCS}
	)
target_link_libraries (leechcraft_azoth_vader
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_vader DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothvadersettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_vader Network Sql Widgets)
