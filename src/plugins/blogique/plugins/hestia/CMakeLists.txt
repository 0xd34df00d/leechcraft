CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_blogique_hestia)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTSQL TRUE)
INCLUDE (${QT_USE_FILE})

INCLUDE_DIRECTORIES (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${BLOGIQUE_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/blogique
	)

SET (HESTIA_SRCS
	accountconfigurationdialog.cpp
	accountconfigurationwidget.cpp
	accountstorage.cpp
	core.cpp
	hestia.cpp
	localblogaccount.cpp
	localbloggingplatform.cpp
	xmlsettingsmanager.cpp
	)

SET (HESTIA_HEADERS
	accountconfigurationdialog.h
	accountstorage.h
	core.h
	hestia.h
	accountconfigurationwidget.h
	localblogaccount.h
	localbloggingplatform.h
	xmlsettingsmanager.h
	)

SET (HESTIA_FORMS
	accountconfigurationdialog.ui
	accountconfigurationwidget.ui
 	)

# SET (HESTIA_RESOURCES blogiquehestiaresources.qrc)

# CreateTrVars ("blogique_hestia" "de;en;es;fr;it;ru_RU;uk_UA" HESTIA_TRANSLATIONS HESTIA_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (HESTIA_MOC_SRCS ${HESTIA_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (HESTIA_UIS_H ${HESTIA_FORMS})
QT4_ADD_RESOURCES (HESTIA_RCCS ${HESTIA_RESOURCES})

# ADD_CUSTOM_COMMAND (OUTPUT
# 	${HESTIA_COMPILED_TRANSLATIONS}
# 	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${HESTIA_TRANSLATIONS}
# 	DEPENDS ${HESTIA_TRANSLATIONS}
# 	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# 	)

ADD_LIBRARY (leechcraft_blogique_hestia SHARED
	${HESTIA_COMPILED_TRANSLATIONS}
	${HESTIA_SRCS}
	${HESTIA_MOC_SRCS}
	${HESTIA_UIS_H}
	${HESTIA_RCCS}
	)

TARGET_LINK_LIBRARIES (leechcraft_blogique_hestia
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

INSTALL (TARGETS leechcraft_blogique_hestia
		DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${HESTIA_COMPILED_TRANSLATIONS}
		DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES blogiquehestiasettings.xml
		DESTINATION ${LC_SETTINGS_DEST})