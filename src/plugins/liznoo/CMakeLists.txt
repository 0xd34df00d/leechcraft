cmake_minimum_required (VERSION 2.8)
project (leechcraft_liznoo)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (Qwt)

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
	xmlsettingsmanager.cpp
	platform/events/platformlayer.cpp
	platform/screen/screenplatformlayer.cpp
	)

set (FBSD_SRCS platform/platformfreebsd.cpp)
set (FBSD_HEADERS platform/platformfreebsd.h)
set (UPOWER_SRCS
	platform/events/platformupower.cpp
	platform/events/dbusthread.cpp
	platform/events/dbusconnector.cpp
	)
set (MAC_SRCS
	platform/events/platformmac.mm
	)
set (WIN32_SRCS
	platform/events/platformwinapi.cpp
	platform/events/fakeqwidgetwinapi.cpp
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
	else ()
		set (SRCS
			${SRCS}
			${UPOWER_SRCS}
			)
	endif ()

	set (SRCS ${SRCS} platform/screen/screenplatformfreedesktop.cpp)
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
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_liznoo SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_liznoo
	${LEECHCRAFT_LIBRARIES}
	${QWT_LIBRARIES}
	${AdditionalLibraries}
	)
install (TARGETS leechcraft_liznoo DESTINATION ${LC_PLUGINS_DEST})
install (FILES liznoosettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_liznoo Widgets)

if (NOT WIN32 AND NOT APPLE)
	FindQtLibs (leechcraft_liznoo DBus)
endif ()
