INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (CLEANWEB_SRCS
	cleanweb.cpp
	core.cpp
	xmlsettingsmanager.cpp
	subscriptionsmanager.cpp
	flashplaceholder.cpp
	flashonclickplugin.cpp
	flashonclickwhitelist.cpp
	blockednetworkreply.cpp
	userfilters.cpp
	userfiltersmodel.cpp
	filter.cpp
	ruleoptiondialog.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	)
SET (CLEANWEB_HEADERS
	cleanweb.h
	core.h
	xmlsettingsmanager.h
	subscriptionsmanager.h
	flashplaceholder.h
	flashonclickplugin.h
	flashonclickwhitelist.h
	blockednetworkreply.h
	userfilters.h
	userfiltersmodel.h
	filter.h
	ruleoptiondialog.h
	wizardgenerator.h
	startupfirstpage.h
	)
SET (CLEANWEB_FORMS
	subscriptionsmanager.ui
	flashplaceholder.ui
	flashonclickwhitelist.ui
	userfilters.ui
	ruleoptiondialog.ui
	subscriptionadddialog.ui
	startupfirstpage.ui
	)
SET (CLEANWEB_RESOURCES
	poshukucleanwebresources.qrc
	)
SET (CLEANWEB_TRANSLATIONS
	leechcraft_poshuku_cleanweb_ru_RU.ts
	leechcraft_poshuku_cleanweb_uk_UA.ts
	leechcraft_poshuku_cleanweb_ar.ts
	leechcraft_poshuku_cleanweb_eo.ts
	)
SET (CLEANWEB_COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_cleanweb_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_cleanweb_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_cleanweb_ar.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_cleanweb_eo.qm
	)
QT4_WRAP_CPP (CLEANWEB_MOC_SRCS ${CLEANWEB_HEADERS})
QT4_WRAP_UI (CLEANWEB_UIS_H ${CLEANWEB_FORMS})
QT4_ADD_RESOURCES (CLEANWEB_RCCS ${CLEANWEB_RESOURCES})
ADD_CUSTOM_COMMAND (OUTPUT
	${CLEANWEB_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${CLEANWEB_TRANSLATIONS}
	DEPENDS ${CLEANWEB_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_poshuku_cleanweb SHARED
	${CLEANWEB_COMPILED_TRANSLATIONS}
	${CLEANWEB_SRCS}
	${CLEANWEB_MOC_SRCS}
	${CLEANWEB_UIS_H}
	${CLEANWEB_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_cleanweb
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_cleanweb DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${CLEANWEB_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES poshukucleanwebsettings.xml DESTINATION ${LC_SETTINGS_DEST})
