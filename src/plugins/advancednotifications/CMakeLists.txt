cmake_minimum_required (VERSION 2.8)
project (leechcraft_advancednotifications)
include (InitLCPlugin OPTIONAL)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	advancednotifications.cpp
	core.cpp
	generalhandler.cpp
	concretehandlerbase.cpp
	systemtrayhandler.cpp
	notificationruleswidget.cpp
	xmlsettingsmanager.cpp
	notificationrule.cpp
	fieldmatch.cpp
	typedmatchers.cpp
	matchconfigdialog.cpp
	visualhandler.cpp
	audiohandler.cpp
	cmdrunhandler.cpp
	enablesoundactionmanager.cpp
	wmurgenthandler.cpp
	rulesmanager.cpp
	quarkproxy.cpp
	actionsmodel.cpp
	qml/visualnotificationsview.cpp
	qml/eventproxyobject.cpp
	qml/actionsproxyobject.cpp
	)

set (FORMS
	notificationruleswidget.ui
	matchconfigdialog.ui
	intmatcherconfigwidget.ui
	stringlikematcherconfigwidget.ui
	)

add_definitions(-DHAVE_QML=1)

set (RESOURCES advancednotificationsresources.qrc)

CreateTrs ("advancednotifications" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_advancednotifications SHARED
	${SRCS}
	${RCCS}
	${UIS_H}
	)

FindQtLibs (leechcraft_advancednotifications Quick)

TARGET_LINK_LIBRARIES (leechcraft_advancednotifications
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_advancednotifications DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES advancednotificationssettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/qml/advancednotifications DESTINATION ${LC_QML_DEST})
INSTALL (DIRECTORY share/sounds DESTINATION ${LC_SHARE_DEST})
