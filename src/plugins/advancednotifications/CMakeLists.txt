cmake_minimum_required (VERSION 2.8)
project (leechcraft_advancednotifications)
include (InitLCPlugin OPTIONAL)

if (USE_QT5)
	find_package (Qt5Quick REQUIRED)
else ()
	set (QT_USE_QTDECLARATIVE TRUE)
	include (${QT_USE_FILE})
endif ()

set (INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
if (USE_QT5)
	set (INCLUDE_DIRS ${INCLUDE_DIRS}
		${Qt5Quick_INCLUDE_DIRS}
		)
endif ()

include_directories (${INCLUDE_DIRS})

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

if (NOT USE_QT5)
	if (NOT LC_NO_MOC)
		QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
	endif (NOT LC_NO_MOC)
	QT4_WRAP_UI (UIS_H ${FORMS})
	QT4_ADD_RESOURCES (RCCS ${RESOURCES})
else ()
	QT5_WRAP_UI (UIS_H ${FORMS})
	QT5_ADD_RESOURCES (RCCS ${RESOURCES})
endif ()

set (ADD_LIBRARY_SET ${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${UIS_H}
	)

if (NOT USE_QT5)
	set (ADD_LIBRARY_SET ${ADD_LIBRARY_SET}
		${MOC_SRCS}
		)
endif ()

ADD_LIBRARY (leechcraft_advancednotifications SHARED
	${ADD_LIBRARY_SET}
	)

if (USE_QT5)
	QT5_USE_MODULES (leechcraft_advancednotifications
		Quick
		)
	TARGET_LINK_LIBRARIES (leechcraft_advancednotifications
		${LEECHCRAFT_LIBRARIES}
		)
else ()
	TARGET_LINK_LIBRARIES (leechcraft_advancednotifications
		${QT_LIBRARIES}
		${LEECHCRAFT_LIBRARIES}
		)
endif ()

INSTALL (TARGETS leechcraft_advancednotifications DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES advancednotificationssettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/qml/advancednotifications DESTINATION ${LC_QML_DEST})
INSTALL (DIRECTORY share/sounds DESTINATION ${LC_SHARE_DEST})
