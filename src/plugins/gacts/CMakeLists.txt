cmake_minimum_required (VERSION 2.8)
project (leechcraft_gacts)
include (InitLCPlugin OPTIONAL)

option (WITH_GACTS_BUNDLED_QXT "Build with bundled Qxt with fixed media keys handling" ON)

if (NOT WITH_GACTS_BUNDLED_QXT)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

	set(QXT_USE_QXTCORE TRUE)
	set(QXT_USE_QXTGUI TRUE)
	find_package (Qxt REQUIRED)
else (NOT WITH_GACTS_BUNDLED_QXT)
	add_definitions (-DBUILD_QXT_GUI)

	if (WIN32)
		set (ADDITIONAL_SOURCES
			3rdparty/qxt/qxtglobalshortcut_win.cpp
			)
	elseif (APPLE)
		set (ADDITIONAL_SOURCES
			3rdparty/qxt/qxtglobalshortcut_mac.cpp
			)
	else (WIN32)
		find_package (X11)
		include_directories (${X11_INCLUDE_DIR})
		
		set (ADDITIONAL_SOURCES
			3rdparty/qxt/qxtglobalshortcut_x11.cpp
			)
	endif (WIN32)
	
	set (ADDITIONAL_SOURCES
		${ADDITIONAL_SOURCES}
		3rdparty/qxt/qxtglobal.cpp
		3rdparty/qxt/qxtglobalshortcut.cpp
		)
	
	include_directories (${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/qxt)
endif (NOT WITH_GACTS_BUNDLED_QXT)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${QXT_INCLUDE_DIRS}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	gacts.cpp
	)

add_library (leechcraft_gacts SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${ADDITIONAL_SOURCES}
	)
target_link_libraries (leechcraft_gacts
	${QT_LIBRARIES}
	${QXT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_gacts DESTINATION ${LC_PLUGINS_DEST})
