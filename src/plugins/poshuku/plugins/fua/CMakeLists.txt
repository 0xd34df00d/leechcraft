INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (FUA_SRCS
	fua.cpp
	settings.cpp
	changer.cpp
	xmlsettingsmanager.cpp
	)
SET (FUA_HEADERS
	fua.h
	settings.h
	changer.h
	xmlsettingsmanager.h
	)
SET (FUA_FORMS
	settings.ui
	changer.ui
	)
SET (FUA_RESOURCES poshukufuaresources.qrc)
SET (FUA_TRANSLATIONS
	leechcraft_poshuku_fua_ru_RU.ts
	leechcraft_poshuku_fua_uk_UA.ts
	leechcraft_poshuku_fua_ar.ts
	)
SET (FUA_COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_fua_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_fua_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_poshuku_fua_ar.qm
	)
QT4_WRAP_CPP (FUA_MOC_SRCS ${FUA_HEADERS})
QT4_WRAP_UI (FUA_UIS_H ${FUA_FORMS})
QT4_ADD_RESOURCES (FUA_RCCS ${FUA_RESOURCES})
ADD_CUSTOM_COMMAND (OUTPUT
	${FUA_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${FUA_TRANSLATIONS}
	DEPENDS ${FUA_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_poshuku_fua SHARED
	${FUA_COMPILED_TRANSLATIONS}
	${FUA_SRCS}
	${FUA_MOC_SRCS}
	${FUA_UIS_H}
	${FUA_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_fua
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_poshuku_fua DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${FUA_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES poshukufuasettings.xml DESTINATION ${LC_SETTINGS_DEST})
