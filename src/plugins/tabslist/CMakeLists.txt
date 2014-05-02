if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_tabslist)

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
	tabslist.cpp
	)
CreateTrs ("tabslist" "en;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

set (TABSLIST_RESOURCES tabslistresources.qrc)
QT4_ADD_RESOURCES (TABSLIST_RCCS ${TABSLIST_RESOURCES})

add_library (leechcraft_tabslist SHARED
	${TABSLIST_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_tabslist
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_tabslist DESTINATION ${LC_PLUGINS_DEST})
