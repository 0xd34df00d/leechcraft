CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_blogique_metida)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTXML TRUE)
SET (QT_USE_QTXMLPATTERNS TRUE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (${BLOGIQUE_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/blogique
	)

SET (METIDA_SRCS
	core.cpp
	frienditemdelegate.cpp
	ljaccount.cpp
	ljaccountconfigurationdialog.cpp
	ljaccountconfigurationwidget.cpp
	ljbloggingplatform.cpp
	ljfriendentry.cpp
	ljprofile.cpp
	ljxmlrpc.cpp
	metida.cpp
	profiletypes.cpp
	profilewidget.cpp
	utils.cpp
	xmlsettingsmanager.cpp
	)

SET (METIDA_HEADERS
	core.h
	frienditemdelegate.h
	ljaccount.h
	ljaccountconfigurationdialog.h
	ljaccountconfigurationwidget.h
	ljbloggingplatform.h
	ljfriendentry.h
	ljprofile.h
	ljxmlrpc.h
	metida.h
	profiletypes.h
	profilewidget.h
	utils.h
	xmlsettingsmanager.h
	)

SET (METIDA_FORMS
	ljaccountconfigurationdialog.ui
	ljaccountconfigurationwidget.ui
	profilewidget.ui
	)

SET (METIDA_RESOURCES blogiquemetidaresources.qrc)

# CreateTrVars ("blogique_metida" "de;en;es;fr;it;ru_RU;uk_UA" METIDA_TRANSLATIONS METIDA_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (METIDA_MOC_SRCS ${METIDA_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (METIDA_UIS_H ${METIDA_FORMS})
QT4_ADD_RESOURCES (METIDA_RCCS ${METIDA_RESOURCES})

# ADD_CUSTOM_COMMAND (OUTPUT
# 	${METIDA_COMPILED_TRANSLATIONS}
# 	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${METIDA_TRANSLATIONS}
# 	DEPENDS ${METIDA_TRANSLATIONS}
# 	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# 	)

ADD_LIBRARY (leechcraft_blogique_metida SHARED
	${METIDA_COMPILED_TRANSLATIONS}
	${METIDA_SRCS}
	${METIDA_MOC_SRCS}
	${METIDA_UIS_H}
	${METIDA_RCCS}
	)

TARGET_LINK_LIBRARIES (leechcraft_blogique_metida
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_blogique_metida
		DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${METIDA_COMPILED_TRANSLATIONS}
		DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES blogiquemetidasettings.xml
		DESTINATION ${LC_SETTINGS_DEST})
