cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_modnok)
include (InitLCPlugin OPTIONAL)

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (MODNOK_SRCS
	modnok.cpp
	xmlsettingsmanager.cpp
	)
set (MODNOK_HEADERS
	modnok.h
	xmlsettingsmanager.h
	)
set (MODNOK_RESOURCES azothmodnokresources.qrc)
CreateTrs ("azoth_modnok" "en;es;ru_RU;uk_UA" MODNOK_COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MODNOK_MOC_SRCS ${MODNOK_HEADERS})
endif (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (MODNOK_RCCS ${MODNOK_RESOURCES})

add_library (leechcraft_azoth_modnok SHARED
	${MODNOK_COMPILED_TRANSLATIONS}
	${MODNOK_SRCS}
	${MODNOK_MOC_SRCS}
	${MODNOK_RCCS}
	)
target_link_libraries (leechcraft_azoth_modnok
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_modnok DESTINATION ${LC_PLUGINS_DEST})
install (PROGRAMS lc_azoth_modnok_latexconvert.sh DESTINATION ${LC_SHARE_DEST}/azoth)
install (FILES azothmodnoksettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${MODNOK_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
