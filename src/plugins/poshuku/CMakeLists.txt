if (NOT QT_USE_FILE)
	cmake_minimum_required (VERSION 2.6)
	if (COMMAND cmake_policy)
		cmake_policy (SET CMP0003 NEW)
	endif ()

	project (leechcraft_poshuku)

	if (NOT CMAKE_MODULE_PATH)
		set (CMAKE_MODULE_PATH "/usr/local/share/leechcraft/cmake;/usr/share/leechcraft/cmake")
	endif ()

	find_package (Boost REQUIRED)
	find_package (Qt4 REQUIRED)
	find_package (LeechCraft REQUIRED)
endif ()

option (ENABLE_IDN "Enable support for Internationalized Domain Names" OFF)

if (ENABLE_IDN)
	add_definitions (-DENABLE_IDN)
endif ()

set (QT_USE_QTSQL TRUE)
set (QT_USE_QTXML TRUE)
set (QT_USE_QTWEBKIT TRUE)
set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	poshuku.cpp
	xmlsettingsmanager.cpp
	core.cpp
	browserwidget.cpp
	customwebview.cpp
	progresslineedit.cpp
	favoritesmodel.cpp
	filtermodel.cpp
	favoritesdelegate.cpp
	favoritestreeview.cpp
	customwebpage.cpp
	historymodel.cpp
	storagebackend.cpp
	sqlstoragebackend.cpp
	sqlstoragebackend_mysql.cpp
	urlcompletionmodel.cpp
	finddialog.cpp
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
	jsproxy.cpp
	externalproxy.cpp
	passwordremember.cpp
	pageformsdata.cpp
	linkhistory.cpp
	cookiesfilter.cpp
	searchtext.cpp
	reloadintervalselector.cpp
	editbookmarkdialog.cpp
	favoriteschecker.cpp
	webpluginfactory.cpp
	browserwidgetsettings.cpp
	hideabletabwidget.cpp
	bookmarkswidget.cpp
	historywidget.cpp
	urlframe.cpp
	importentityhandler.cpp
	platformplugin.cpp
	urleditbuttonsmanager.cpp
	webpagesslwatcher.cpp
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
	)
set (RESOURCES poshukuresources.qrc)

CreateTrs ("poshuku" "ar;be;de;en;eo;es;fr;it;pl;ru_RU;tt;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku" "en;ru_RU" "${SRCS}" "${FORMS}" "poshukusettings.xml")

QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_poshuku SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

if (ENABLE_IDN)
	set (IDN_LIB "idn")
endif ()

target_link_libraries (leechcraft_poshuku
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${IDN_LIB}
	)
install (TARGETS leechcraft_poshuku DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukusettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY installed/poshuku/ DESTINATION ${LC_INSTALLEDMANIFEST_DEST}/poshuku)
install (DIRECTORY interfaces DESTINATION include/leechcraft)

set (POSHUKU_INCLUDE_DIR ${CURRENT_SOURCE_DIR})

option (ENABLE_POSHUKU_AUTOSEARCH "Build autosearch plugin for Poshuku browser" ON)
option (ENABLE_POSHUKU_CLEANWEB "Build content filter plugin for Poshuku browser" ON)
option (ENABLE_POSHUKU_FATAPE "Build FatApe, support for GreaseMonkey userscripts" ON)
option (ENABLE_POSHUKU_FILESCHEME "Build support of local file browsing for Poshuku browser" ON)
option (ENABLE_POSHUKU_FUA "Build user agent customization plugin for Poshuku browser" ON)
option (ENABLE_POSHUKU_KEYWORDS "Build support of url keywords for Poshuku browser" ON)
option (ENABLE_POSHUKU_ONLINEBOOKMARKS "Build support of online bookmark synchronization for Poshuku browser" ON)
option (ENABLE_POSHUKU_SPEEDDIAL "Build Speed Dial support module" ON)

if (ENABLE_POSHUKU_AUTOSEARCH)
	add_subdirectory (plugins/autosearch)
endif ()
if (ENABLE_POSHUKU_CLEANWEB)
	add_subdirectory (plugins/cleanweb)
endif ()
if (ENABLE_POSHUKU_KEYWORDS)
	add_subdirectory (plugins/keywords)
endif ()
if (ENABLE_POSHUKU_FATAPE)
	add_subdirectory (plugins/fatape)
endif ()
if (ENABLE_POSHUKU_FUA)
	add_subdirectory (plugins/fua)
endif ()
if (ENABLE_POSHUKU_FILESCHEME)
	add_subdirectory (plugins/filescheme)
endif ()
if (ENABLE_POSHUKU_ONLINEBOOKMARKS)
	add_subdirectory (plugins/onlinebookmarks)
endif ()
if (ENABLE_POSHUKU_SPEEDDIAL)
	add_subdirectory (plugins/speeddial)
endif ()
