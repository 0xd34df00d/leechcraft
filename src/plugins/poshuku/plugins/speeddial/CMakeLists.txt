include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (SPEEDDIAL_SRCS
	speeddial.cpp
	viewhandler.cpp
	imagecache.cpp
	customsitesmanager.cpp
	xmlsettingsmanager.cpp
	)
set (SPEEDDIAL_RESOURCES
	)
CreateTrs ("poshuku_speeddial" "en;ru_RU" SPEEDDIAL_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_speeddial" "en;ru_RU" "${SPEEDDIAL_SRCS}" "" "poshukuspeeddialsettings.xml")

QT4_ADD_RESOURCES (SPEEDDIAL_RCCS ${SPEEDDIAL_RESOURCES})
add_library (leechcraft_poshuku_speeddial SHARED
	${SPEEDDIAL_COMPILED_TRANSLATIONS}
	${SPEEDDIAL_SRCS}
	${SPEEDDIAL_MOC_SRCS}
	${SPEEDDIAL_RCCS}
	)
target_link_libraries (leechcraft_poshuku_speeddial
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_speeddial DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukuspeeddialsettings.xml DESTINATION ${LC_SETTINGS_DEST})
