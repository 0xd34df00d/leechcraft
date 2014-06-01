cmake_minimum_required (VERSION 2.8)
project (leechcraft_lhtr)
include (InitLCPlugin OPTIONAL)

option (WITH_LHTR_HTML "Build extended support for (invalid) HTML" ON)

if (WITH_LHTR_HTML)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (LibTidy REQUIRED)
	set (ADDITIONAL_LIBRARES ${LIBTIDY_LIBRARIES})
	add_definitions (-DWITH_HTMLTIDY)
endif (WITH_LHTR_HTML)

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LIBTIDY_INCLUDE_DIRS}
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
QT4_WRAP_UI (UIS_H ${FORMS})
CreateTrs ("lhtr" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("lhtr" "en;ru_RU" "${SRCS}" "${FORMS}" "lhtrsettings.xml")

set (LHTR_RESOURCES lhtrresources.qrc)
QT4_ADD_RESOURCES (LHTR_RCCS ${LHTR_RESOURCES})

add_library (leechcraft_lhtr SHARED
	${LHTR_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_lhtr
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${ADDITIONAL_LIBRARES}
	)
install (TARGETS leechcraft_lhtr DESTINATION ${LC_PLUGINS_DEST})
install (FILES lhtrsettings.xml DESTINATION ${LC_SETTINGS_DEST})
