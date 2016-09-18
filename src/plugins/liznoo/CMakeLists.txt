cmake_minimum_required (VERSION 2.8)
project (leechcraft_liznoo)
include (InitLCPlugin OPTIONAL)

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
	platformobjects.cpp
	quarkmanager.cpp
	xmlsettingsmanager.cpp
	platform/battery/batteryplatform.cpp
	platform/events/platformlayer.cpp
	platform/poweractions/platform.cpp
	platform/screen/screenplatform.cpp
	)

set (FBSD_SRCS
	platform/battery/freebsdplatform.cpp
	platform/poweractions/freebsd.cpp
	platform/events/platformfreebsd.cpp
	)

set (LINUX_SRCS
	platform/common/dbusthread.cpp
	platform/poweractions/pmutils.cpp
	)

set (LOGIND_SRCS
	platform/logind/logindconnector.cpp
	)

set (UPOWER_SRCS
	platform/battery/upowerplatform.cpp
	platform/upower/upowerconnector.cpp
	platform/poweractions/upower.cpp
	)

set (MAC_SRCS
	platform/battery/macplatform.mm
	platform/events/platformmac.mm
	)

set (WIN32_SRCS
	platform/battery/winapiplatform.cpp
	platform/events/platformwinapi.cpp
	platform/winapi/fakeqwidgetwinapi.cpp
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
			${LINUX_SRCS}
			${UPOWER_SRCS}
			${LOGIND_SRCS}
			)
	endif ()

	set (SRCS
		${SRCS}
		platform/screen/freedesktop.cpp
		)
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
install (DIRECTORY share/qml5/liznoo DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_liznoo Concurrent Widgets)

if (NOT WIN32 AND NOT APPLE)
	FindQtLibs (leechcraft_liznoo DBus)
endif ()
