cmake_minimum_required (VERSION 2.8)
project (leechcraft_cpuload)
include (InitLCPlugin OPTIONAL)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	cpuload.cpp
	backend.cpp
	backendproxy.cpp
	cpuloadproxyobj.cpp
	structures.cpp
	)

set (PLATFORM_LIBS)
if (APPLE)
	set (SRCS ${SRCS} macbackend.mm)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set (SRCS ${SRCS} linuxbackend.cpp)
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
	set (SRCS ${SRCS} freebsdbackend.cpp)
	set (PLATFORM_LIBS kvm)
else ()
	message (STATUS "CpuLoad: unsupported platform")
endif ()

CreateTrs("cpuload" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("cpuload" "en;ru_RU" "${SRCS}" "${FORMS}" "")

file(GLOB QML_FILES "share/qml5/cpuload/*.qml")
CreateQmlTrsUpTarget("cpuload" "en;ru_RU" "${QML_FILES}" "share/qml5/cpuload/CpuLoadQuark.qml.settings")

add_library (leechcraft_cpuload SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_cpuload
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${PLATFORM_LIBS}
	)
install (TARGETS leechcraft_cpuload DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml5/cpuload DESTINATION ${LC_QML5_DEST})

CreateTrsQml("cpuload" "en;ru_RU" leechcraft_cpuload "share/qml5/cpuload")

FindQtLibs (leechcraft_cpuload Quick)
