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

if (APPLE)
	set (SRCS ${SRCS} macbackend.mm)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set (SRCS ${SRCS} linuxbackend.cpp)
else ()
	message (STATUS "CpuLoad: unsupported platform")
endif ()

#CreateTrs("cpuload" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("cpuload" "en;ru_RU" "${SRCS}" "${FORMS}" "")

add_library (leechcraft_cpuload SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_cpuload
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_cpuload DESTINATION ${LC_PLUGINS_DEST})
install (DIRECTORY share/qml/cpuload DESTINATION ${LC_QML_DEST})
install (DIRECTORY share/qml5/cpuload DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_cpuload Quick)
