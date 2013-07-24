if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_seekthru)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

set (QT_USE_QTXML TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
)
set (SRCS
	seekthru.cpp
	core.cpp
	description.cpp
	xmlsettingsmanager.cpp
	searcherslist.cpp
	findproxy.cpp
	searchhandler.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	tagsasker.cpp
	)
set (HEADERS
	seekthru.h
	core.h
	description.h
	xmlsettingsmanager.h
	searcherslist.h
	findproxy.h
	searchhandler.h
	wizardgenerator.h
	startupfirstpage.h
	tagsasker.h
	)
set (TRANSLATIONS
	leechcraft_seekthru_ru_RU.ts
	leechcraft_seekthru_uk_UA.ts
	leechcraft_seekthru_ar.ts
	leechcraft_seekthru_be.ts
	leechcraft_seekthru_de.ts
	leechcraft_seekthru_en.ts
	leechcraft_seekthru_eo.ts
	leechcraft_seekthru_es.ts
	leechcraft_seekthru_fr.ts
	leechcraft_seekthru_pl.ts
	)
set (FORMS
	searcherslist.ui
	startupfirstpage.ui
	tagsasker.ui
	)
set (RESOURCES
	seekthruresources.qrc
	)
set (COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_be.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_de.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_en.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_eo.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_es.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_fr.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_seekthru_pl.qm
	)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif ()
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_custom_command (OUTPUT
	${COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
	DEPENDS ${TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
add_library (leechcraft_seekthru SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_seekthru
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_seekthru DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES seekthrusettings.xml DESTINATION ${LC_SETTINGS_DEST})
