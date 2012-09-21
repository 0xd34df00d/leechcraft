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
	userfilters.cpp
	userfiltersmodel.cpp
	filter.cpp
	ruleoptiondialog.cpp
	wizardgenerator.cpp
	startupfirstpage.cpp
	subscriptionadddialog.cpp
	lineparser.cpp
	regexp.cpp
	)
SET (CLEANWEB_HEADERS
	cleanweb.h
	core.h
	xmlsettingsmanager.h
	subscriptionsmanager.h
	flashplaceholder.h
	flashonclickplugin.h
	flashonclickwhitelist.h
	userfilters.h
	userfiltersmodel.h
	filter.h
	ruleoptiondialog.h
	wizardgenerator.h
	startupfirstpage.h
	subscriptionadddialog.h
	lineparser.h
	regexp.h
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
CreateTrs ("poshuku_cleanweb" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" CLEANWEB_COMPILED_TRANSLATIONS)

OPTION (USE_POSHUKU_CLEANWEB_PCRE "Use PCRE regular expression engine for AdBlock+ filter matching instead of Qt's one (recommended since QRegExps are slow)." ON)
IF (USE_POSHUKU_CLEANWEB_PCRE)
	SET (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	FIND_PACKAGE (PCRE REQUIRED)
	MESSAGE (STATUS "Found PCRE includes: ${PCRE_INCLUDE_DIR}")
	MESSAGE (STATUS "Found PCRE libraries: ${PCRE_LIBRARIES}")
	IF (PCRE_INCLUDE_DIR AND PCRE_LIBRARIES)
		MESSAGE (STATUS "Found PCRE, support for AdBlock+ separator-matching will be built")
	ELSE (PCRE_INCLUDE_DIR AND PCRE_LIBRARIES)
		MESSAGE (FATAL_ERROR "PCRE support isn't available")
	ENDIF (PCRE_INCLUDE_DIR AND PCRE_LIBRARIES)

	ADD_DEFINITIONS (-DUSE_PCRE)
	INCLUDE_DIRECTORIES (${PCRE_INCLUDE_DIR})
ENDIF (USE_POSHUKU_CLEANWEB_PCRE)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (CLEANWEB_MOC_SRCS ${CLEANWEB_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (CLEANWEB_UIS_H ${CLEANWEB_FORMS})
QT4_ADD_RESOURCES (CLEANWEB_RCCS ${CLEANWEB_RESOURCES})
ADD_LIBRARY (leechcraft_poshuku_cleanweb SHARED
	${CLEANWEB_COMPILED_TRANSLATIONS}
	${CLEANWEB_SRCS}
	${CLEANWEB_MOC_SRCS}
	${CLEANWEB_UIS_H}
	${CLEANWEB_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_cleanweb
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${PCRE_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poshuku_cleanweb DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${CLEANWEB_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES poshukucleanwebsettings.xml DESTINATION ${LC_SETTINGS_DEST})
