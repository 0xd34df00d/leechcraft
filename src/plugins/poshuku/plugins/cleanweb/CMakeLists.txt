include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (CLEANWEB_SRCS
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
set (CLEANWEB_FORMS
	subscriptionsmanager.ui
	flashplaceholder.ui
	flashonclickwhitelist.ui
	userfilters.ui
	ruleoptiondialog.ui
	subscriptionadddialog.ui
	startupfirstpage.ui
	)
set (CLEANWEB_RESOURCES
	poshukucleanwebresources.qrc
	)
CreateTrs ("poshuku_cleanweb" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" CLEANWEB_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("poshuku_cleanweb" "en;ru_RU" "${CLEANWEB_SRCS}" "${CLEANWEB_FORMS}" "poshukucleanwebsettings.xml")

option (USE_POSHUKU_CLEANWEB_PCRE "Use PCRE regular expression engine for AdBlock+ filter matching instead of Qt's one (recommended since QRegExps are slow)." ON)
if (USE_POSHUKU_CLEANWEB_PCRE)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (PCRE REQUIRED)
	message (STATUS "Found PCRE includes: ${PCRE_INCLUDE_DIR}")
	message (STATUS "Found PCRE libraries: ${PCRE_LIBRARIES}")
	if (PCRE_INCLUDE_DIR AND PCRE_LIBRARIES)
		message (STATUS "Found PCRE, support for AdBlock+ separator-matching will be built")
	else ()
		message (FATAL_ERROR "PCRE support isn't available")
	endif ()

	add_definitions (-DUSE_PCRE)
	include_directories (${PCRE_INCLUDE_DIR})
endif ()

QT4_WRAP_UI (CLEANWEB_UIS_H ${CLEANWEB_FORMS})
QT4_ADD_RESOURCES (CLEANWEB_RCCS ${CLEANWEB_RESOURCES})
add_library (leechcraft_poshuku_cleanweb SHARED
	${CLEANWEB_COMPILED_TRANSLATIONS}
	${CLEANWEB_SRCS}
	${CLEANWEB_UIS_H}
	${CLEANWEB_RCCS}
	)
target_link_libraries (leechcraft_poshuku_cleanweb
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	${PCRE_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_cleanweb DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${CLEANWEB_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES poshukucleanwebsettings.xml DESTINATION ${LC_SETTINGS_DEST})
