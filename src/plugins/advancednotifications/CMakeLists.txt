cmake_minimum_required (VERSION 3.3)
project (leechcraft_advancednotifications)
include (InitLCPlugin NO_POLICY_SCOPE)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	advancednotifications.cpp
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
	visualnotificationsview.cpp
	eventproxyobject.cpp
	actionsproxyobject.cpp
	audiothememanager.cpp
	unhandlednotificationskeeper.cpp
	addfrommisseddialog.cpp
	)
set (FORMS
	notificationruleswidget.ui
	matchconfigdialog.ui
	boolmatcherconfigwidget.ui
	intmatcherconfigwidget.ui
	stringlikematcherconfigwidget.ui
	addfrommisseddialog.ui
	)

set (RESOURCES advancednotificationsresources.qrc)

CreateTrs ("advancednotifications" "en;es;ro;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("advancednotifications" "en;ru_RU" "${SRCS}" "${FORMS}" "advancednotificationssettings.xml")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_advancednotifications SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_advancednotifications
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_advancednotifications DESTINATION ${LC_PLUGINS_DEST})
install (FILES advancednotificationssettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml5/advancednotifications DESTINATION ${LC_QML5_DEST})
install (DIRECTORY share/sounds DESTINATION ${LC_SHARE_DEST})

option (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE "Enable Dolle, OS X notifications backend" OFF)

if (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE)
	add_subdirectory (plugins/dolle)
endif ()

FindQtLibs (leechcraft_advancednotifications QuickWidgets)
