cmake_minimum_required (VERSION 2.8)
project (leechcraft_monocle)
include (InitLCPlugin OPTIONAL)

option (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
option (WITH_DOCS_INSTALL "Install generated documentation (if WITH_DOCS is set)" OFF)
if (WITH_DOCS)
	find_package (Doxygen REQUIRED)

	set (DOXYDIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../doc/doxygen/monocle/")
	set (DOXYFILE "${DOXYDIR}/Doxyfile")

	add_custom_target (doc_monocle ALL
		"sed" "-i" "s/^PROJECT_NUMBER.*/PROJECT_NUMBER = '${LEECHCRAFT_VERSION}'/" "${DOXYFILE}"
		COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE}"
		COMMAND "mv" "-f" "${DOXYDIR}/out" "${CMAKE_CURRENT_BINARY_DIR}/out"
		WORKING_DIRECTORY "${DOXYDIR}"
	)
	if (WITH_DOCS_INSTALL)
		install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-monocle-${LEECHCRAFT_VERSION}")
	endif ()
endif ()

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	monocle.cpp
	documenttab.cpp
	core.cpp
	pagegraphicsitem.cpp
	filewatcher.cpp
	tocwidget.cpp
	presenterwidget.cpp
	pagesview.cpp
	xmlsettingsmanager.cpp
	pixmapcachemanager.cpp
	recentlyopenedmanager.cpp
	choosebackenddialog.cpp
	defaultbackendmanager.cpp
	docstatemanager.cpp
	docinfodialog.cpp
	bookmarkswidget.cpp
	bookmarksmanager.cpp
	bookmark.cpp
	thumbswidget.cpp
	pageslayoutmanager.cpp
	textsearchhandler.cpp
	formmanager.cpp
	arbitraryrotationwidget.cpp
	annmanager.cpp
	annitem.cpp
	annwidget.cpp
	anntreedelegate.cpp
	linkitem.cpp
	linksmanager.cpp
	coreloadproxy.cpp
	converteddoccleaner.cpp
	searchtabwidget.cpp
	documentbookmarksmanager.cpp
	)
set (FORMS
	documenttab.ui
	tocwidget.ui
	choosebackenddialog.ui
	docinfodialog.ui
	bookmarkswidget.ui
	thumbswidget.ui
	arbitraryrotationwidget.ui
	annwidget.ui
	searchtabwidget.ui
	)
set (RESOURCES monocleresources.qrc)
CreateTrs ("monocle" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("monocle" "en;ru_RU" "${SRCS}" "${FORMS}" "monoclesettings.xml")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

set (UTIL_SRCS
	util/monocle/textdocumentadapter.cpp
	)
add_library (leechcraft_monocle_util STATIC
	${UTIL_SRCS}
	)
set_target_properties(leechcraft_monocle_util PROPERTIES POSITION_INDEPENDENT_CODE True)
FindQtLibs (leechcraft_monocle_util Widgets)

add_library (leechcraft_monocle SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_monocle
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_monocle DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES monoclesettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_monocle Concurrent PrintSupport Widgets Xml)

option (ENABLE_MONOCLE_DIK "Enable MOBI backend for Monocle" ON)
option (ENABLE_MONOCLE_FXB "Enable FictionBook backend for Monocle" ON)
option (ENABLE_MONOCLE_MU "Enable PDF backend for Monocle using the mupdf library" OFF)
option (ENABLE_MONOCLE_PDF "Enable PDF backend for Monocle using the Poppler library" ON)
option (ENABLE_MONOCLE_POSTRUS "Enable PostScript backend for Monocle using the libspectre library" ON)
option (ENABLE_MONOCLE_SEEN "Enable DjVu backend for Monocle using the DjVu library" ON)

if (ENABLE_MONOCLE_DIK)
	add_subdirectory (plugins/dik)
endif ()

if (ENABLE_MONOCLE_FXB)
	add_subdirectory (plugins/fxb)
endif ()

if (ENABLE_MONOCLE_MU)
	add_subdirectory (plugins/mu)
endif ()

if (ENABLE_MONOCLE_PDF)
	add_subdirectory (plugins/pdf)
endif ()

if (ENABLE_MONOCLE_POSTRUS)
	add_subdirectory (plugins/postrus)
endif ()

if (ENABLE_MONOCLE_SEEN)
	add_subdirectory (plugins/seen)
endif ()
