cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_isterique)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ISTERIQUE_SRCS
	isterique.cpp
	xmlsettingsmanager.cpp
	)
set (ISTERIQUE_HEADERS
	isterique.h
	xmlsettingsmanager.h
	)
set (ISTERIQUE_RESOURCES
	isterique.qrc
	)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (ISTERIQUE_MOC_SRCS ${ISTERIQUE_HEADERS})
endif ()
QT4_ADD_RESOURCES (ISTERIQUE_RCCS ${ISTERIQUE_RESOURCES})
CreateTrs ("azoth_isterique" "en;ru_RU;uk_UA" ISTERIQUE_COMPILED_TRANSLATIONS)

add_library (leechcraft_azoth_isterique SHARED
	${ISTERIQUE_COMPILED_TRANSLATIONS}
	${ISTERIQUE_SRCS}
	${ISTERIQUE_MOC_SRCS}
	${ISTERIQUE_RCCS}
	)
target_link_libraries (leechcraft_azoth_isterique
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_isterique DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ISTERIQUE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES azothisteriquesettings.xml DESTINATION ${LC_SETTINGS_DEST})
