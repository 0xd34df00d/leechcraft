cmake_minimum_required (VERSION 2.8)
project (leechcraft_poshuku)
include (InitLCPlugin OPTIONAL)

option (ENABLE_IDN "Enable support for Internationalized Domain Names" OFF)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

if (ENABLE_IDN)
	FIND_PACKAGE (IDN REQUIRED)
	add_definitions (-DENABLE_IDN)
else ()
	unset (IDN_LIBRARIES)
endif ()

include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${IDN_INCLUDE_DIR}
	)
set (SRCS
	poshuku.cpp
	xmlsettingsmanager.cpp
	core.cpp
	browserwidget.cpp
	progresslineedit.cpp
	favoritesmodel.cpp
	filtermodel.cpp
	favoritesdelegate.cpp
	favoritestreeview.cpp
	historymodel.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	sqlstoragebackend_mysql.cpp
	urlcompletionmodel.cpp
	screenshotsavedialog.cpp
	cookieseditdialog.cpp
	cookieseditmodel.cpp
	sourceviewer.cpp
	htmlhighlighter.cpp
	xbelparser.cpp
	xbelgenerator.cpp
	pluginmanager.cpp
	historyfiltermodel.cpp
	proxyobject.cpp
	passwordremember.cpp
	pageformsdata.cpp
	cookiesfilter.cpp
	searchtext.cpp
	reloadintervalselector.cpp
	editbookmarkdialog.cpp
	favoriteschecker.cpp
	browserwidgetsettings.cpp
	hideabletabwidget.cpp
	bookmarkswidget.cpp
	historywidget.cpp
	urlframe.cpp
	importentity.cpp
	urleditbuttonsmanager.cpp
	featurepermnotification.cpp
	zoomer.cpp
	browserwidgetsettingshandler.cpp
	linkopenmodifier.cpp
	)
set (FORMS
	browserwidget.ui
	screenshotsavedialog.ui
	cookieseditdialog.ui
	sourceviewer.ui
	passwordremember.ui
	searchtext.ui
	reloadintervalselector.ui
	editbookmarkdialog.ui
	hideabletabwidget.ui
	bookmarkswidget.ui
	historywidget.ui
	urlframe.ui
	featurepermnotification.ui
	)
set (RESOURCES poshukuresources.qrc)

CreateTrs ("poshuku" "ar;be;de;en;eo;es;fr;it;pl;ru_RU;tt;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku" "en;ru_RU" "${SRCS}" "${FORMS}" "poshukusettings.xml")

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_poshuku SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

target_link_libraries (leechcraft_poshuku
	${LEECHCRAFT_LIBRARIES}
	${IDN_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukusettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY installed/poshuku/ DESTINATION ${LC_INSTALLEDMANIFEST_DEST}/poshuku)
install (DIRECTORY interfaces DESTINATION include/leechcraft)

FindQtLibs (leechcraft_poshuku Network PrintSupport Sql Xml)

set (POSHUKU_INCLUDE_DIR ${CURRENT_SOURCE_DIR})

option (ENABLE_POSHUKU_AUTOSEARCH "Build autosearch plugin for Poshuku browser" ON)
option (ENABLE_POSHUKU_CLEANWEB "Build content filter plugin for Poshuku browser" ON)
option (ENABLE_POSHUKU_DCAC "Build DC/AC for inverting colors on web pages" ON)
option (ENABLE_POSHUKU_FATAPE "Build FatApe, support for GreaseMonkey userscripts" ON)
option (ENABLE_POSHUKU_FILESCHEME "Build support of local file browsing for Poshuku browser" ON)
option (ENABLE_POSHUKU_FOC "Build support for Flash-on-Click for Poshuku browser" ON)
option (ENABLE_POSHUKU_FUA "Build user agent customization plugin for Poshuku browser" ON)
option (ENABLE_POSHUKU_KEYWORDS "Build support of url keywords for Poshuku browser" ON)
option (ENABLE_POSHUKU_ONLINEBOOKMARKS "Build support of online bookmark synchronization for Poshuku browser" ON)
option (ENABLE_POSHUKU_QRD "Build QRd for showing the URL of a web page as a QR code" OFF)
option (ENABLE_POSHUKU_SPEEDDIAL "Build Speed Dial support module" ON)

option (ENABLE_POSHUKU_WEBENGINEVIEW "Build WebEngine-based backend" OFF)
option (ENABLE_POSHUKU_WEBKITVIEW "Build WebKit-based backend" ON)

if (ENABLE_POSHUKU_AUTOSEARCH)
	add_subdirectory (plugins/autosearch)
endif ()
if (ENABLE_POSHUKU_CLEANWEB)
	add_subdirectory (plugins/cleanweb)
endif ()
if (ENABLE_POSHUKU_DCAC)
	add_subdirectory (plugins/dcac)
endif ()
if (ENABLE_POSHUKU_KEYWORDS)
	add_subdirectory (plugins/keywords)
endif ()
if (ENABLE_POSHUKU_FATAPE)
	add_subdirectory (plugins/fatape)
endif ()
if (ENABLE_POSHUKU_FILESCHEME)
	add_subdirectory (plugins/filescheme)
endif ()
if (ENABLE_POSHUKU_FOC)
	add_subdirectory (plugins/foc)
endif ()
if (ENABLE_POSHUKU_FUA)
	add_subdirectory (plugins/fua)
endif ()
if (ENABLE_POSHUKU_ONLINEBOOKMARKS)
	add_subdirectory (plugins/onlinebookmarks)
endif ()
if (ENABLE_POSHUKU_QRD)
	add_subdirectory (plugins/qrd)
endif ()
if (ENABLE_POSHUKU_SPEEDDIAL)
	add_subdirectory (plugins/speeddial)
endif ()

if (ENABLE_POSHUKU_WEBENGINEVIEW)
	add_subdirectory (plugins/webengineview)
endif ()
if (ENABLE_POSHUKU_WEBKITVIEW)
	add_subdirectory (plugins/webkitview)
endif ()
