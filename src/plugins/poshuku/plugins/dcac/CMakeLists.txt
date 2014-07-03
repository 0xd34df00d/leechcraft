include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (DCAC_SRCS
	dcac.cpp
	inverteffect.cpp
	viewsmanager.cpp
	xmlsettingsmanager.cpp
	)
set (DCAC_RESOURCES
	)
CreateTrs ("poshuku_dcac" "en;ru_RU" DCAC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_dcac" "en;ru_RU" "${DCAC_SRCS}" "" "poshukudcacsettings.xml")

QT4_ADD_RESOURCES (DCAC_RCCS ${DCAC_RESOURCES})
add_library (leechcraft_poshuku_dcac SHARED
	${DCAC_COMPILED_TRANSLATIONS}
	${DCAC_SRCS}
	${DCAC_MOC_SRCS}
	${DCAC_RCCS}
	)
target_link_libraries (leechcraft_poshuku_dcac
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_dcac DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukudcacsettings.xml DESTINATION ${LC_SETTINGS_DEST})
