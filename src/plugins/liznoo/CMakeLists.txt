CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_liznoo)
INCLUDE (InitLCPlugin OPTIONAL)

SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
FIND_PACKAGE (Qwt6)

IF (NOT WIN32 AND NOT APPLE)
	SET (QT_USE_QTDBUS TRUE)
ENDIF (NOT WIN32 AND NOT APPLE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${Qwt6_INCLUDE_DIR}
	)
SET (SRCS
	liznoo.cpp
	batteryinfo.cpp
	batteryhistory.cpp
	batteryhistorydialog.cpp
	platformlayer.cpp
	xmlsettingsmanager.cpp
	)
SET (HEADERS
	liznoo.h
	batteryinfo.h
	batteryhistory.h
	batteryhistorydialog.h
	platformlayer.h
	xmlsettingsmanager.h
	)

SET (FBSD_SRCS platformfreebsd.cpp)
SET (FBSD_HEADERS platformfreebsd.h)
SET (UPOWER_SRCS
	platformupower.cpp
	dbusthread.cpp
	dbusconnector.cpp
	)
SET (UPOWER_HEADERS
	platformupower.h
	dbusthread.h
	dbusconnector.h
	)
SET (MAC_SRCS
	platformmac.mm
	)
SET (MAC_HEADERS
	platformmac.h
	)
SET (WIN32_SRCS
	platformwinapi.cpp
	fakeqwidgetwinapi.cpp
	)
SET (WIN32_HEADERS
	platformwinapi.h
	fakeqwidgetwinapi.h
	)
# Default implementation using UPower
IF (APPLE)
	SET (SRCS ${SRCS} ${MAC_SRCS})
	SET (HEADERS ${HEADERS} ${MAC_HEADERS})
ELSEIF (NOT WIN32)
	IF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		SET (SRCS
			${SRCS}
			${FBSD_SRCS}
			)
		SET (HEADERS
			${HEADERS}
			${FBSD_HEADERS}
			)
	ELSE (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		SET (SRCS
			${SRCS}
			${UPOWER_SRCS}
			)
		SET (HEADERS
			${HEADERS}
			${UPOWER_HEADERS}
			)
	ENDIF (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
# Windows implementation using WinAPI
ELSE (APPLE)
	SET (SRCS
		${SRCS}
		${WIN32_SRCS}
		)
	SET (HEADERS
		${HEADERS}
		${WIN32_HEADERS}
		)
ENDIF (APPLE)

SET (FORMS
	batteryhistorydialog.ui
	)
SET (RESOURCES liznooresources.qrc)
CreateTrs ("liznoo" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("liznoo" "en;ru_RU" "${SRCS};${FBSD_SRCS};${UPOWER_SRCS};${WIN32_SRCS}" "${FORMS}" "liznoosettings.xml")
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_liznoo SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_liznoo
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${Qwt6_Qt4_LIBRARY}
	)
INSTALL (TARGETS leechcraft_liznoo DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES liznoosettings.xml DESTINATION ${LC_SETTINGS_DEST})
