cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_hili)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (HILI_SRCS
	hili.cpp
	xmlsettingsmanager.cpp
	)
set (HILI_HEADERS
	hili.h
	xmlsettingsmanager.h
	)
set (HILI_RESOURCES azothhiliresources.qrc)
CreateTrs ("azoth_hili" "en;es;ru_RU;uk_UA" HILI_COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (HILI_MOC_SRCS ${HILI_HEADERS})
endif ()
QT4_ADD_RESOURCES (HILI_RCCS ${HILI_RESOURCES})

add_library (leechcraft_azoth_hili SHARED
	${HILI_COMPILED_TRANSLATIONS}
	${HILI_SRCS}
	${HILI_MOC_SRCS}
	${HILI_RCCS}
	)
target_link_libraries (leechcraft_azoth_hili
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_hili RUNTIME DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothhilisettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${HILI_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
