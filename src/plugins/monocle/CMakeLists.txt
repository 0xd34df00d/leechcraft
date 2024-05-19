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

add_library (leechcraft-monocle-util SHARED
	util/monocle/html2doc.cpp
	util/monocle/imghandler.cpp
	util/monocle/formatutils.cpp
	util/monocle/linksbuilder.cpp
	util/monocle/pagelink.cpp
	util/monocle/resourcedtextdocument.cpp
	util/monocle/tablehandler.cpp
	util/monocle/textdocumentadapter.cpp
	util/monocle/textdocumentformatconfig.cpp
	util/monocle/tocbuilder.cpp
	util/monocle/types.cpp
	)
FindQtLibs (leechcraft-monocle-util Widgets Xml)
set_property (TARGET leechcraft-monocle-util PROPERTY SOVERSION ${LC_SOVERSION})
if (WITH_MONOCLE_IMAGEMAGICK)
	find_package (PkgConfig)
	pkg_check_modules (MagickPP REQUIRED IMPORTED_TARGET Magick++)
	target_link_libraries (leechcraft-monocle-util PkgConfig::MagickPP)
	target_compile_definitions (leechcraft-monocle-util PRIVATE WITH_IMAGEMAGICK)
endif ()
install (TARGETS leechcraft-monocle-util DESTINATION ${LIBDIR} RENAME leechcraft-monocle-util${LC_LIBSUFFIX})

LC_DEFINE_PLUGIN (
	SRCS
		components/actions/export.cpp
		components/actions/rotatemenu.cpp
		components/actions/zoomer.cpp
		components/layout/positiontracker.cpp
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
		linkactionexecutor.cpp
		coreloadproxy.cpp
		searchtabwidget.cpp
		documentbookmarksmanager.cpp
		navigationhistory.cpp
		pagenumlabel.cpp
		smoothscroller.cpp
		common.cpp
	SETTINGS monoclesettings.xml
	QT_COMPONENTS Concurrent PrintSupport Widgets Xml
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
