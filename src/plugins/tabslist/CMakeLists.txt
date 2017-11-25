cmake_minimum_required (VERSION 3.3)
project (leechcraft_tabslist)
include (InitLCPlugin)

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
QtAddResources (TABSLIST_RCCS ${TABSLIST_RESOURCES})

add_library (leechcraft_tabslist SHARED
	${TABSLIST_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	)
target_link_libraries (leechcraft_tabslist
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_tabslist DESTINATION ${LC_PLUGINS_DEST})

FindQtLibs (leechcraft_tabslist Widgets)
