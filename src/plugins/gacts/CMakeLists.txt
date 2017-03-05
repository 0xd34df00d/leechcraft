cmake_minimum_required (VERSION 2.8)
project (leechcraft_gacts)
include (InitLCPlugin OPTIONAL)

add_definitions (-DBUILD_QXT_GUI)

if (WIN32)
	set (ADDITIONAL_SOURCES
		3rdparty/qxt/qxtglobalshortcut_win.cpp
		)
elseif (APPLE)
	set (ADDITIONAL_SOURCES
		3rdparty/qxt/qxtglobalshortcut_mac.cpp
		)
else ()
	find_package (X11)
	include_directories (${X11_INCLUDE_DIR})

	set (ADDITIONAL_SOURCES
		3rdparty/qxt/qxtglobalshortcut_x11.cpp
		)
endif ()

set (ADDITIONAL_SOURCES
	${ADDITIONAL_SOURCES}
	3rdparty/qxt/qxtglobal.cpp
	3rdparty/qxt/qxtglobalshortcut.cpp
	)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	gacts.cpp
	)

if (APPLE)
	set (ADDITIONAL_LIBS "-framework CoreFoundation;-framework Carbon")
endif ()

CreateTrs("gacts" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("gacts" "en;ru_RU" "${SRCS}" "" "")

add_library (leechcraft_gacts SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${ADDITIONAL_SOURCES}
	)
target_link_libraries (leechcraft_gacts
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
install (TARGETS leechcraft_gacts DESTINATION ${LC_PLUGINS_DEST})

if (UNIX AND NOT APPLE)
	FindQtLibs (leechcraft_gacts X11Extras)
endif ()
