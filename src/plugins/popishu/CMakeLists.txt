if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_popishu)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
find_package (QScintilla)
if (NOT QSCINTILLA_FOUND)
	message (ERROR "QScintilla not found, unable to build Popishu :(")
endif ()

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${QSCINTILLA_INCLUDE_DIR}
	)
set (SRCS
	popishu.cpp
	editorpage.cpp
	xmlsettingsmanager.cpp
	replacedialog.cpp
	)
set (RESOURCES
	popishuresources.qrc
	)
set (FORMS
	editorpage.ui
	replacedialog.ui
	)
CreateTrs ("popishu" "de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("popishu" "en;ru_RU" "${SRCS}" "${FORMS}" "popishusettings.xml")

QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_popishu SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${HEADERS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_popishu
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${QSCINTILLA_LIBRARIES}
	)
install (TARGETS leechcraft_popishu DESTINATION ${LC_PLUGINS_DEST})
install (FILES popishusettings.xml DESTINATION ${LC_SETTINGS_DEST})
