cmake_minimum_required (VERSION 2.8)
project (leechcraft_gacts)
include (InitLCPlugin OPTIONAL)

option (WITH_GACTS_BUNDLED_QXT "Build with bundled Qxt with fixed media keys handling" ON)

if (NOT WITH_GACTS_BUNDLED_QXT)
	if (USE_QT5)
		message (FATAL_ERROR "Building Gacts with Qxt and Qt5 is not supported (as there are no
				public releases of Qt5-enabled Qxt yet. Consider disabling WITH_GACTS_BUNDLED_QXT option.")
	endif ()

	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

	set(QXT_USE_QXTCORE TRUE)
	set(QXT_USE_QXTGUI TRUE)
	find_package (Qxt REQUIRED)
else ()
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

	unset (QXT_INCLUDE_DIRS)
	unset (QXT_LIBRARIES)
endif ()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${QXT_INCLUDE_DIRS}
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
	${QXT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBS}
	)
install (TARGETS leechcraft_gacts DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_gacts X11Extras)
