include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (WEBKITVIEW_SRCS
	customwebpage.cpp
	customwebview.cpp
	externalproxy.cpp
	interceptadaptor.cpp
	jsproxy.cpp
	linkhistory.cpp
	pluginmanager.cpp
	settingsglobalhandler.cpp
	settingsinstancehandler.cpp
	sslstatedialog.cpp
	webkitview.cpp
	webpagesslwatcher.cpp
	webpluginfactory.cpp
	webviewrendersettingshandler.cpp
	webviewsmoothscroller.cpp
	webviewsslwatcherhandler.cpp
	xmlsettingsmanager.cpp
	)
set (WEBKITVIEW_FORMS
	sslstatedialog.ui
	)
set (WEBKITVIEW_RESOURCES)
CreateTrs ("poshuku_webkitview" "en;ru_RU" WEBKITVIEW_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_webkitview" "en;ru_RU" "${WEBKITVIEW_SRCS}" "${WEBKITVIEW_FORMS}" "poshukuwebkitviewsettings.xml")

QtWrapUi (WEBKITVIEW_UIS_H ${WEBKITVIEW_FORMS})
QtAddResources (WEBKITVIEW_RCCS ${WEBKITVIEW_RESOURCES})
add_library (leechcraft_poshuku_webkitview SHARED
	${WEBKITVIEW_COMPILED_TRANSLATIONS}
	${WEBKITVIEW_SRCS}
	${WEBKITVIEW_UIS_H}
	${WEBKITVIEW_RCCS}
	)
target_link_libraries (leechcraft_poshuku_webkitview
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_webkitview DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukuwebkitviewsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_webkitview PrintSupport WebKitWidgets)
