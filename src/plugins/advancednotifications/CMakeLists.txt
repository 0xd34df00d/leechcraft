CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_advancednotifications)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTDECLARATIVE TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
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
	qml/visualnotificationsview.cpp
	qml/eventproxyobject.cpp
	qml/actionsproxyobject.cpp
	)
SET (HEADERS
	advancednotifications.h
	core.h
	generalhandler.h
	concretehandlerbase.h
	systemtrayhandler.h
	notificationruleswidget.h
	xmlsettingsmanager.h
	notificationrule.h
	fieldmatch.h
	typedmatchers.h
	matchconfigdialog.h
	visualhandler.h
	audiohandler.h
	cmdrunhandler.h
	enablesoundactionmanager.h
	wmurgenthandler.h
	rulesmanager.h
	quarkproxy.h
	qml/visualnotificationsview.h
	qml/eventproxyobject.h
	qml/actionsproxyobject.h
	)
SET (FORMS
	notificationruleswidget.ui
	matchconfigdialog.ui
	intmatcherconfigwidget.ui
	stringlikematcherconfigwidget.ui
	)

ADD_DEFINITIONS(-DHAVE_QML=1)

SET (RESOURCES advancednotificationsresources.qrc)

CreateTrs ("advancednotifications" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_advancednotifications SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${RCCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_advancednotifications
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_advancednotifications DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES advancednotificationssettings.xml DESTINATION ${LC_SETTINGS_DEST})
INSTALL (DIRECTORY share/qml/advancednotifications DESTINATION ${LC_QML_DEST})
INSTALL (DIRECTORY share/sounds DESTINATION ${LC_SHARE_DEST})
