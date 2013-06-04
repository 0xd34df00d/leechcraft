cmake_minimum_required (VERSION 2.8)
project (leechcraft_dumbeep)
include (InitLCPlugin OPTIONAL)

option (DUMBEEP_WITH_PHONON "Build with support for Phonon backend" OFF)

if (DUMBEEP_WITH_PHONON)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

	if (NOT WIN32)
		find_package (Phonon)
	endif (NOT WIN32)

	if (WIN32)
		message (STATUS "Using Qt's Phonon on Windows")
		set (QT_USE_PHONON TRUE)
	else (WIN32)
		if (NOT PHONON_FOUND)
			message (WARNING "KDE Phonon not found, trying Qt's one instead...")
			set (QT_USE_PHONON TRUE)
		else (NOT PHONON_FOUND)
			message (STATUS "Found KDE Phonon! Woohoo!")
			set (QT_PHONON_INCLUDES ${PHONON_INCLUDES})
			set (QT_PHONON_LIBS ${PHONON_LIBS})
			add_definitions (-DHAVE_KDE_PHONON)
		endif (NOT PHONON_FOUND)
	endif (WIN32)

	add_definitions (-DWITH_PHONON)
endif (DUMBEEP_WITH_PHONON)

include (${QT_USE_FILE})
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
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QT_PHONON_LIBS}
	)
install (TARGETS leechcraft_dumbeep DESTINATION ${LC_PLUGINS_DEST})
install (FILES dumbeepsettings.xml DESTINATION ${LC_SETTINGS_DEST})
