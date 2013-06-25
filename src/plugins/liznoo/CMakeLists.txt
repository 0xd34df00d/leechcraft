cmake_minimum_required (VERSION 2.8)
project (leechcraft_liznoo)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (Qwt)

if (NOT WIN32 AND NOT APPLE)
	set (QT_USE_QTDBUS TRUE)
endif ()
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QWT_INCLUDE_DIRS}
	)
set (SRCS
	liznoo.cpp
	batteryinfo.cpp
	batteryhistory.cpp
	batteryhistorydialog.cpp
	platformlayer.cpp
	xmlsettingsmanager.cpp
	)
set (HEADERS
	liznoo.h
	batteryinfo.h
	batteryhistory.h
	batteryhistorydialog.h
	platformlayer.h
	xmlsettingsmanager.h
	)

set (FBSD_SRCS platformfreebsd.cpp)
set (FBSD_HEADERS platformfreebsd.h)
set (UPOWER_SRCS
	platformupower.cpp
	dbusthread.cpp
	dbusconnector.cpp
	)
set (UPOWER_HEADERS
	platformupower.h
	dbusthread.h
	dbusconnector.h
	)
set (MAC_SRCS
	platformmac.mm
	)
set (MAC_HEADERS
	platformmac.h
	)
set (WIN32_SRCS
	platformwinapi.cpp
	fakeqwidgetwinapi.cpp
	)
set (WIN32_HEADERS
	platformwinapi.h
	fakeqwidgetwinapi.h
	)
# Default implementation using UPower
if (APPLE)
	set (SRCS ${SRCS} ${MAC_SRCS})
	set (HEADERS ${HEADERS} ${MAC_HEADERS})
	set (AdditionalLibraries "-framework IOKit;-framework CoreFoundation")
elseif (NOT WIN32)
	if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
		set (SRCS
			${SRCS}
			${FBSD_SRCS}
			)
		set (HEADERS
			${HEADERS}
			${FBSD_HEADERS}
			)
	else ()
		set (SRCS
			${SRCS}
			${UPOWER_SRCS}
			)
		set (HEADERS
			${HEADERS}
			${UPOWER_HEADERS}
			)
	endif ()
# Windows implementation using WinAPI
else ()
	set (SRCS
		${SRCS}
		${WIN32_SRCS}
		)
	set (HEADERS
		${HEADERS}
		${WIN32_HEADERS}
		)
endif ()

set (FORMS
	batteryhistorydialog.ui
	)
set (RESOURCES liznooresources.qrc)
CreateTrs ("liznoo" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("liznoo" "en;ru_RU" "${SRCS};${FBSD_SRCS};${UPOWER_SRCS};${WIN32_SRCS}" "${FORMS}" "liznoosettings.xml")
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_liznoo SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_liznoo
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QWT_LIBRARIES}
	${AdditionalLibraries}
	)
install (TARGETS leechcraft_liznoo DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES liznoosettings.xml DESTINATION ${LC_SETTINGS_DEST})
