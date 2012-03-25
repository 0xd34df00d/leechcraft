CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_vader)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

SET (VADER_SRCS
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
SET (VADER_HEADERS
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
SET (VADER_FORMS
	mrimaccountconfigwidget.ui
	smsdialog.ui
	vcarddialog.ui
	)
SET (VADER_RESOURCES azothvaderresources.qrc)
CreateTrVars ("azoth_vader" "en;es;ru_RU;uk_UA" VADER_TRANSLATIONS VADER_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (VADER_MOC_SRCS ${VADER_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (VADER_UIS_H ${VADER_FORMS})
QT4_ADD_RESOURCES (VADER_RCCS ${VADER_RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${VADER_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${VADER_TRANSLATIONS}
	DEPENDS ${VADER_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_vader SHARED
	${VADER_COMPILED_TRANSLATIONS}
	${VADER_SRCS}
	${VADER_MOC_SRCS}
	${VADER_UIS_H}
	${VADER_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_vader
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_azoth_vader DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${VADER_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES azothvadersettings.xml DESTINATION ${LC_SETTINGS_DEST})
