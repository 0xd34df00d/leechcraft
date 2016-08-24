include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FOC_SRCS
	flashplaceholder.cpp
	flashonclickplugin.cpp
	flashonclickwhitelist.cpp
	foc.cpp
	xmlsettingsmanager.cpp
	)
set (FOC_FORMS
	flashplaceholder.ui
	flashonclickwhitelist.ui
	)
set (FOC_RESOURCES
	)
#CreateTrs ("poshuku_foc" "en;ru_RU" FOC_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_foc" "en;ru_RU" "${FOC_SRCS}" "${FOC_FORMS}" "poshukufocsettings.xml")

QtWrapUi (FOC_UIS_H ${FOC_FORMS})
QtAddResources (FOC_RCCS ${FOC_RESOURCES})
add_library (leechcraft_poshuku_foc SHARED
	${FOC_COMPILED_TRANSLATIONS}
	${FOC_SRCS}
	${FOC_RCCS}
	${FOC_UIS_H}
	)
target_link_libraries (leechcraft_poshuku_foc
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_foc DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukufocsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_foc Widgets WebKitWidgets)
