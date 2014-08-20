include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FUA_SRCS
	fua.cpp
	settings.cpp
	changer.cpp
	xmlsettingsmanager.cpp
	)
set (FUA_FORMS
	settings.ui
	changer.ui
	)
set (FUA_RESOURCES poshukufuaresources.qrc)
CreateTrs ("poshuku_fua" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" FUA_COMPILED_TRANSLATIONS)
CreateTrsUpTarget("poshuku_fua" "en;ru_RU" "${FUA_SRCS}" "${FUA_FORMS}" "poshukufuasettings.xml")

QtWrapUi (FUA_UIS_H ${FUA_FORMS})
QtAddResources (FUA_RCCS ${FUA_RESOURCES})
add_library (leechcraft_poshuku_fua SHARED
	${FUA_COMPILED_TRANSLATIONS}
	${FUA_SRCS}
	${FUA_UIS_H}
	${FUA_RCCS}
	)
target_link_libraries (leechcraft_poshuku_fua
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_fua DESTINATION ${LC_PLUGINS_DEST})
install (FILES poshukufuasettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_poshuku_fua Widgets)
