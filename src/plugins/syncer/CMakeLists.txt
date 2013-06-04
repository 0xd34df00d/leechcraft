if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif (COMMAND cmake_policy)

	project (leechcraft_syncer)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif (NOT CMAKE_MODULE_PATH)

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif (NOT QT_USE_FILE)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	syncer.cpp
	core.cpp
	datastoragebase.cpp
	datastorageserver.cpp
	serverconnection.cpp
	serverchainhandler.cpp
	xmlsettingsmanager.cpp
	)
set (HEADERS
	syncer.h
	core.h
	datastoragebase.h
	datastorageserver.h
	serverconnection.h
	serverchainhandler.h
	xmlsettingsmanager.h
	)
set (TRANSLATIONS
	leechcraft_syncer_ru_RU.ts
	leechcraft_syncer_en.ts
	)
set (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_syncer_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_syncer_en.qm
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)

add_custom_command (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_library (leechcraft_syncer SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	)
target_link_libraries (leechcraft_syncer
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_syncer DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES syncersettings.xml DESTINATION ${LC_SETTINGS_DEST})