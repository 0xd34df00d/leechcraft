cmake_minimum_required (VERSION 3.10)
project (wkplugins)
include (InitLCPlugin NO_POLICY_SCOPE)

add_library (leechcraft_wkplugins_qtplugin STATIC staticplugin.cpp)
set_target_properties (leechcraft_wkplugins_qtplugin PROPERTIES COMPILE_FLAGS "-DQT_STATICPLUGIN")
FindQtLibs (leechcraft_wkplugins_qtplugin WebKitWidgets)

get_target_property (QMAKE_EXECUTABLE Qt5::qmake LOCATION)
execute_process (COMMAND ${QMAKE_EXECUTABLE} -query QT_INSTALL_HEADERS
	OUTPUT_VARIABLE HEADERS_PATH
	OUTPUT_STRIP_TRAILING_WHITESPACE
	)
QT5_WRAP_CPP (WRAPPED_QHEADERS ${HEADERS_PATH}/QtWebKit/qwebkitplatformplugin.h)

LC_DEFINE_PLUGIN (
	SRCS
		wkplugins.cpp
		notificationsext.cpp
		spellcheckext.cpp
		xmlsettingsmanager.cpp
		${WRAPPED_QHEADERS}
	SETTINGS wkpluginssettings.xml
	QT_COMPONENTS WebKitWidgets
	LINK_LIBRARIES leechcraft_wkplugins_qtplugin
	)
