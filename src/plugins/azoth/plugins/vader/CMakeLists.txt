cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_vader)
include (InitLCPlugin OPTIONAL)

option (ENABLE_AZOTH_VADER_VERBOSE_LOG "Debug protocol messages & similar stuff with increased verbosity" OFF)

if (ENABLE_AZOTH_VADER_VERBOSE_LOG)
	add_definitions (-DPROTOCOL_LOGGING)
endif (ENABLE_AZOTH_VADER_VERBOSE_LOG)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

set (VADER_SRCS
	vader.cpp
	core.cpp
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
set (VADER_HEADERS
	vader.h
	core.h
	mrimprotocol.h
	mrimaccount.h
	mrimaccountconfigwidget.h
	mrimbuddy.h
	mrimmessage.h
	vaderutil.h
	groupmanager.h
	smsdialog.h
	selfavatarfetcher.h
	xmlsettingsmanager.h
	vcarddialog.h
	proto/headers.h
	proto/conversions.h
	proto/connection.h
	proto/packet.h
	proto/packetfactory.h
	proto/packetextractor.h
	proto/exceptions.h
	proto/halfpacket.h
	proto/balancer.h
	proto/typingmanager.h
	)
set (VADER_FORMS
	mrimaccountconfigwidget.ui
	smsdialog.ui
	vcarddialog.ui
	)
set (VADER_RESOURCES azothvaderresources.qrc)
CreateTrs ("azoth_vader" "en;es;ru_RU;uk_UA" VADER_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (VADER_MOC_SRCS ${VADER_HEADERS})
endif (NOT LC_NO_MOC)
QT4_WRAP_UI (VADER_UIS_H ${VADER_FORMS})
QT4_ADD_RESOURCES (VADER_RCCS ${VADER_RESOURCES})

add_library (leechcraft_azoth_vader SHARED
	${VADER_COMPILED_TRANSLATIONS}
	${VADER_SRCS}
	${VADER_MOC_SRCS}
	${VADER_UIS_H}
	${VADER_RCCS}
	)
target_link_libraries (leechcraft_azoth_vader
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_azoth_vader DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${VADER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES azothvadersettings.xml DESTINATION ${LC_SETTINGS_DEST})
