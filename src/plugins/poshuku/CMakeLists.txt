cmake_minimum_required (VERSION 3.10)
project (leechcraft_poshuku)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_IDN "Enable support for Internationalized Domain Names" OFF)

if (ENABLE_IDN)
	add_definitions (-DENABLE_IDN)

	find_package (PkgConfig)
	pkg_check_modules (IDN REQUIRED IMPORTED_TARGET libidn)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
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
		dummywebview.cpp
	SETTINGS poshukusettings.xml
	RESOURCES poshukuresources.qrc
	QT_COMPONENTS Network PrintSupport Sql Xml
	LINK_LIBRARIES $<$<BOOL:${ENABLE_IDN}>:PkgConfig::IDN>
	)
set (RESOURCES poshukuresources.qrc)

SUBPLUGIN (AUTOSEARCH "Build autosearch plugin for Poshuku browser")
SUBPLUGIN (CLEANWEB "Build content filter plugin for Poshuku browser")
SUBPLUGIN (DCAC "Build DC/AC for inverting colors on web pages" OFF)
SUBPLUGIN (FATAPE "Build FatApe, support for GreaseMonkey userscripts")
SUBPLUGIN (FILESCHEME "Build support of local file browsing for Poshuku browser")
SUBPLUGIN (FUA "Build user agent customization plugin for Poshuku browser")
SUBPLUGIN (KEYWORDS "Build support of url keywords for Poshuku browser")
SUBPLUGIN (ONLINEBOOKMARKS "Build support of online bookmark synchronization for Poshuku browser")
SUBPLUGIN (QRD "Build QRd for showing the URL of a web page as a QR code" OFF)
SUBPLUGIN (SPEEDDIAL "Build Speed Dial support module")

SUBPLUGIN (WEBENGINEVIEW "Build WebEngine-based backend" ON)
SUBPLUGIN (WEBKITVIEW "Build WebKit-based backend" OFF)
