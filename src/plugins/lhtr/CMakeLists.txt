cmake_minimum_required (VERSION 3.3)
project (leechcraft_lhtr)
include (InitLCPlugin NO_POLICY_SCOPE)

option (WITH_LHTR_HTML "Build extended support for (invalid) HTML" ON)
option (USE_LIBTIDY_HTML5 "Use the libtidy-html5 fork instead of libtidy" ON)

if (WITH_LHTR_HTML)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (LibTidy REQUIRED)
	set (ADDITIONAL_LIBRARES ${LIBTIDY_LIBRARIES})
	add_definitions (-DWITH_HTMLTIDY)

	if (USE_LIBTIDY_HTML5)
		add_definitions (-DUSE_LIBTIDY_HTML5)
	endif ()
endif (WITH_LHTR_HTML)

include_directories (SYSTEM
	${Boost_INCLUDE_DIR}
	${LIBTIDY_INCLUDE_DIRS}
	)
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	lhtr.cpp
	richeditorwidget.cpp
	hyperlinkdialog.cpp
	imagedialog.cpp
	finddialog.cpp
	inserttabledialog.cpp
	xmlsettingsmanager.cpp
	htmlhighlighter.cpp
	imagecollectiondialog.cpp
	imageinfosmodel.cpp
	)
set (FORMS
	richeditorwidget.ui
	hyperlinkdialog.ui
	imagedialog.ui
	finddialog.ui
	inserttabledialog.ui
	imagecollectiondialog.ui
	)
QtWrapUi (UIS_H ${FORMS})
CreateTrs ("lhtr" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("lhtr" "en;ru_RU" "${SRCS}" "${FORMS}" "lhtrsettings.xml")

set (LHTR_RESOURCES lhtrresources.qrc)
QtAddResources (LHTR_RCCS ${LHTR_RESOURCES})

add_library (leechcraft_lhtr SHARED
	${LHTR_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_lhtr
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBRARES}
	)
install (TARGETS leechcraft_lhtr DESTINATION ${LC_PLUGINS_DEST})
install (FILES lhtrsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_lhtr Network WebKitWidgets)
