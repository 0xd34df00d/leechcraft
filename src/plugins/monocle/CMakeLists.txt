cmake_minimum_required (VERSION 3.10)
project (monocle)
include (InitLCPlugin NO_POLICY_SCOPE)

option (WITH_MONOCLE_IMAGEMAGICK "Use imagemagick for enhancing images quality" OFF)

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

if (WITH_MONOCLE_IMAGEMAGICK)
	find_package (PkgConfig)
	pkg_check_modules (MagickPP REQUIRED IMPORTED_TARGET Magick++)
endif ()

add_util_library (monocle-util
	SRCS
		util/monocle/documentsignals.cpp
		util/monocle/html2doc.cpp
		util/monocle/imghandler.cpp
		util/monocle/formatutils.cpp
		util/monocle/linksbuilder.cpp
		util/monocle/pagelink.cpp
		util/monocle/resourcedtextdocument.cpp
		util/monocle/tablehandler.cpp
		util/monocle/textboxesdetector.cpp
		util/monocle/textdocumentadapter.cpp
		util/monocle/textdocumentformatconfig.cpp
		util/monocle/tocbuilder.cpp
		util/monocle/types.cpp
	USES Widgets Xml
	DEPENDS $<$<BOOL:${WITH_MONOCLE_IMAGEMAGICK}>:PkgConfig::MagickPP>
	DEFINES $<$<BOOL:${WITH_MONOCLE_IMAGEMAGICK}>:WITH_IMAGEMAGICK>
	)

LC_DEFINE_PLUGIN (
	SRCS
		components/actions/documentactions.cpp
		components/actions/export.cpp
		components/actions/rotatemenu.cpp
		components/actions/toolbaractions.cpp
		components/actions/zoomer.cpp
		components/layout/pageslayoutmanager.cpp
		components/layout/positions.cpp
		components/layout/scenepositiontracker.cpp
		components/layout/viewpositiontracker.cpp
		components/navigation/bookmark.cpp
		components/navigation/bookmarksstorage.cpp
		components/navigation/documentbookmarksmodel.cpp
		components/navigation/navigationhistory.cpp
		components/navigation/navigator.cpp
		components/services/docstatemanager.cpp
		components/services/documentloader.cpp
		components/services/linkactionexecutor.cpp
		components/services/pixmapcachemanager.cpp
		components/services/recentlyopenedmanager.cpp
		components/viewitems/annitem.cpp
		components/viewitems/interactionhandlers.cpp
		components/viewitems/linkitem.cpp
		components/viewitems/nondragclickfilter.cpp
		components/viewitems/pagegraphicsitem.cpp
		components/viewitems/pagescontextmenuhandler.cpp
		components/viewitems/pagesview.cpp
		components/widgets/dock.cpp
		monocle.cpp
		documenttab.cpp
		core.cpp
		filewatcher.cpp
		tocwidget.cpp
		presenterwidget.cpp
		choosebackenddialog.cpp
		defaultbackendmanager.cpp
		docinfodialog.cpp
		bookmarkswidget.cpp
		thumbswidget.cpp
		textsearchhandler.cpp
		formmanager.cpp
		arbitraryrotationwidget.cpp
		annmanager.cpp
		annwidget.cpp
		anntreedelegate.cpp
		searchtabwidget.cpp
		pagenumlabel.cpp
		smoothscroller.cpp
		common.cpp
	SETTINGS monoclesettings.xml
	QT_COMPONENTS Concurrent PrintSupport Sql Widgets Xml
	LINK_LIBRARIES leechcraft-monocle-util
	INSTALL_SHARE
	)

SUBPLUGIN (BOOP "Enable EPub backend for Monocle")
SUBPLUGIN (DIK "Enable MOBI backend for Monocle")
SUBPLUGIN (FXB "Enable FictionBook backend for Monocle")
SUBPLUGIN (MU "Enable PDF backend for Monocle using the mupdf library" OFF)
SUBPLUGIN (PDF "Enable PDF backend for Monocle using the Poppler library")
SUBPLUGIN (POSTRUS "Enable PostScript backend for Monocle using the libspectre library")
SUBPLUGIN (SEEN "Enable DjVu backend for Monocle using the DjVu library")
