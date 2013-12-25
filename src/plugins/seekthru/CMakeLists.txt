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
set (FORMS
	searcherslist.ui
	startupfirstpage.ui
	tagsasker.ui
	)
set (RESOURCES
	seekthruresources.qrc
	)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

CreateTrs("seekthru" "ar;be;de;en;eo;es;fr;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("seekthru" "en;ru_RU" "${SRCS}" "${FORMS}" "seekthrusettings.xml")

add_library (leechcraft_seekthru SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_seekthru
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_seekthru DESTINATION ${LC_PLUGINS_DEST})
install (FILES seekthrusettings.xml DESTINATION ${LC_SETTINGS_DEST})
