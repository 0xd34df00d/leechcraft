cmake_minimum_required (VERSION 2.8)
project (leechcraft_dumbeep)
include (InitLCPlugin OPTIONAL)

option (DUMBEEP_WITH_PHONON "Build with support for Phonon backend" OFF)

if (DUMBEEP_WITH_PHONON)
	if (USE_QT5)
		message (FATAL_ERROR "Building Dumbeep with Phonon is not supported on Qt5, disable DUMBEEP_WITH_PHONON option.")
	endif ()

	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

	if (NOT WIN32)
		find_package (Phonon)
	endif ()

	if (WIN32)
		message (STATUS "Using Qt's Phonon on Windows")
		set (QT_USE_PHONON TRUE)
	else ()
		if (NOT PHONON_FOUND)
			message (WARNING "KDE Phonon not found, trying Qt's one instead...")
			set (QT_USE_PHONON TRUE)
		else ()
			message (STATUS "Found KDE Phonon! Woohoo!")
			set (QT_PHONON_INCLUDES ${PHONON_INCLUDES})
			set (QT_PHONON_LIBS ${PHONON_LIBS})
			add_definitions (-DHAVE_KDE_PHONON)
		endif ()
	endif ()

	add_definitions (-DWITH_PHONON)
endif ()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	dumbeep.cpp
	xmlsettingsmanager.cpp
	)

add_library (leechcraft_dumbeep SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_dumbeep
	${LEECHCRAFT_LIBRARIES}
	${QT_PHONON_LIBS}
	)
install (TARGETS leechcraft_dumbeep DESTINATION ${LC_PLUGINS_DEST})
install (FILES dumbeepsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_dumbeep Core)
