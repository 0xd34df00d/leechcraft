cmake_minimum_required (VERSION 3.10)
project (monocle)
include (InitLCPlugin NO_POLICY_SCOPE)

option (WITH_DOCS "Enable building documentation (requires Doxygen)" OFF)
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
	install (DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/out/html" DESTINATION "share/doc/leechcraft-monocle-${LEECHCRAFT_VERSION}")
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		monocle.cpp
		documenttab.cpp
		core.cpp
		pagegraphicsitem.cpp
		filewatcher.cpp
		tocwidget.cpp
		presenterwidget.cpp
		pagesview.cpp
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
		navigationhistory.cpp
		pagenumlabel.cpp
		smoothscroller.cpp
		common.cpp
		util/monocle/textdocumentadapter.cpp
	SETTINGS monoclesettings.xml
	QT_COMPONENTS Concurrent PrintSupport Widgets Xml
	INSTALL_SHARE
	)

SUBPLUGIN (DIK "Enable MOBI backend for Monocle")
SUBPLUGIN (FXB "Enable FictionBook backend for Monocle")
SUBPLUGIN (MU "Enable PDF backend for Monocle using the mupdf library" OFF)
SUBPLUGIN (PDF "Enable PDF backend for Monocle using the Poppler library")
SUBPLUGIN (POSTRUS "Enable PostScript backend for Monocle using the libspectre library")
SUBPLUGIN (SEEN "Enable DjVu backend for Monocle using the DjVu library")
