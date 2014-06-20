cmake_minimum_required (VERSION 2.8)
project (leechcraft_advancednotifications)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTDECLARATIVE TRUE)
include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
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
	boolmatcherconfigwidget.ui
	intmatcherconfigwidget.ui
	stringlikematcherconfigwidget.ui
	)

add_definitions(-DHAVE_QML=1)

set (RESOURCES advancednotificationsresources.qrc)

CreateTrs ("advancednotifications" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("advancednotifications" "en;ru_RU" "${SRCS}" "${FORMS}" "advancednotificationssettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_advancednotifications SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${UIS_H}
	)
target_link_libraries (leechcraft_advancednotifications
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_advancednotifications DESTINATION ${LC_PLUGINS_DEST})
install (FILES advancednotificationssettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/qml/advancednotifications DESTINATION ${LC_QML_DEST})
install (DIRECTORY share/sounds DESTINATION ${LC_SHARE_DEST})

option (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE "Enable Dolle, OS X notifications backend" OFF)

if (ENABLE_ADVANCEDNOTIFICATIONS_DOLLE)
	add_subdirectory (plugins/dolle)
endif ()
