include_directories (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
set (FUA_SRCS
	fua.cpp
	settings.cpp
	changer.cpp
	xmlsettingsmanager.cpp
	)
set (FUA_HEADERS
	fua.h
	settings.h
	changer.h
	xmlsettingsmanager.h
	)
set (FUA_FORMS
	settings.ui
	changer.ui
	)
set (FUA_RESOURCES poshukufuaresources.qrc)
CreateTrs ("poshuku_fua" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" FUA_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (FUA_MOC_SRCS ${FUA_HEADERS})
endif ()
QT4_WRAP_UI (FUA_UIS_H ${FUA_FORMS})
QT4_ADD_RESOURCES (FUA_RCCS ${FUA_RESOURCES})
add_library (leechcraft_poshuku_fua SHARED
	${FUA_COMPILED_TRANSLATIONS}
	${FUA_SRCS}
	${FUA_MOC_SRCS}
	${FUA_UIS_H}
	${FUA_RCCS}
	)
target_link_libraries (leechcraft_poshuku_fua
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_poshuku_fua RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${FUA_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES poshukufuasettings.xml DESTINATION ${LC_SETTINGS_DEST})
