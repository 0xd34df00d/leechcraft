INCLUDE_DIRECTORIES (${POSHUKU_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR})
SET (WYFV_SRCS
	wyfv.cpp
	core.cpp
	xmlsettingsmanager.cpp
	wyfvplugin.cpp
	player.cpp
	playerfactory.cpp
	abstractplayercreator.cpp
	youtubeplayer.cpp
	vkontakteruplayer.cpp
	relatedwidget.cpp
	relateditem.cpp
	)
SET (WYFV_HEADERS
	wyfv.h
	core.h
	xmlsettingsmanager.h
	wyfvplugin.h
	player.h
	playerfactory.h
	abstractplayercreator.h
	youtubeplayer.h
	vkontakteruplayer.h
	relatedwidget.h
	relateditem.h
	related.h
	)
SET (WYFV_FORMS
	player.ui
	relateditem.ui
	)
SET (WYFV_RESOURCES
	poshukuwyfvresources.qrc
	)
CreateTrVars ("poshuku_wyfv" "ar;de;en;fr;it;ru_RU;uk_UA" WYFV_TRANSLATIONS WYFV_COMPILED_TRANSLATIONS)

IF (NOT LC_NO_MOC)
	QT4_WRAP_CPP (WYFV_MOC_SRCS ${WYFV_HEADERS})
ENDIF (NOT LC_NO_MOC)
QT4_WRAP_UI (WYFV_UIS_H ${WYFV_FORMS})
QT4_ADD_RESOURCES (WYFV_RCCS ${WYFV_RESOURCES})
ADD_CUSTOM_COMMAND (OUTPUT
	${WYFV_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${WYFV_TRANSLATIONS}
	DEPENDS ${WYFV_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_poshuku_wyfv SHARED
	${WYFV_COMPILED_TRANSLATIONS}
	${WYFV_SRCS}
	${WYFV_MOC_SRCS}
	${WYFV_UIS_H}
	${WYFV_RCCS}
	)
TARGET_LINK_LIBRARIES (leechcraft_poshuku_wyfv
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_poshuku_wyfv DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${WYFV_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
INSTALL (FILES poshukuwyfvsettings.xml DESTINATION ${LC_SETTINGS_DEST})
