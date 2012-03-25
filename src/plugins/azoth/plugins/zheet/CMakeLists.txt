CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_azoth_zheet)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
FIND_PACKAGE (Libmsn REQUIRED)
FIND_PACKAGE (OpenSSL REQUIRED)

INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LIBMSN_INCLUDE_DIR}
	${OPENSSL_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)

SET (ZHEET_SRCS
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
SET (ZHEET_HEADERS
	zheet.h
	core.h
	msnprotocol.h
	msnaccount.h
	callbacks.h
	zheetutil.h
	msnaccountconfigwidget.h
	msnbuddyentry.h
	msnmessage.h
	sbmanager.h
	groupmanager.h
	manageblacklistdialog.h
	transfermanager.h
	transferjob.h
	)
SET (ZHEET_FORMS
	msnaccountconfigwidget.ui
	manageblacklistdialog.ui
	)
SET (ZHEET_RESOURCES azothzheetresources.qrc)
CreateTrVars ("azoth_zheet" "en;es;ru_RU;uk_UA" ZHEET_TRANSLATIONS ZHEET_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (ZHEET_MOC_SRCS ${ZHEET_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (ZHEET_UIS_H ${ZHEET_FORMS})
QT4_ADD_RESOURCES (ZHEET_RCCS ${ZHEET_RESOURCES})

ADD_CUSTOM_COMMAND (OUTPUT
	${ZHEET_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${ZHEET_TRANSLATIONS}
	DEPENDS ${ZHEET_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_zheet SHARED
	${ZHEET_COMPILED_TRANSLATIONS}
	${ZHEET_SRCS}
	${ZHEET_MOC_SRCS}
	${ZHEET_UIS_H}
	${ZHEET_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_zheet
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${LIBMSN_LIBRARIES}
	${OPENSSL_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_azoth_zheet DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${ZHEET_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
#INSTALL (FILES azothzheetsettings.xml DESTINATION ${LC_SETTINGS_DEST})
