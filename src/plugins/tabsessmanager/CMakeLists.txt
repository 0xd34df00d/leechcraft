if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_tabsessmanager)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	tabsessmanager.cpp
	restoresessiondialog.cpp
	recinfo.cpp
	)
set (FORMS
	restoresessiondialog.ui
	)
CreateTrs ("tabsessmanager" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("tabsessmanager" "en;ru_RU" "${SRCS}" "${FORMS}" "")

QT4_WRAP_UI (UIS_H ${FORMS})

add_library (leechcraft_tabsessmanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_tabsessmanager
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_tabsessmanager DESTINATION ${LC_PLUGINS_DEST})
