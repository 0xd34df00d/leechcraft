cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_astrality)
include (InitLCPlugin)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package (TelepathyQt4 REQUIRED)

if (NOT WIN32)
	set (QT_USE_QTDBUS TRUE)
endif ()
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${TELEPATHY_QT4_INCLUDE_DIR}
	${TELEPATHY_QT4_INCLUDE_DIR}/../
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (ASTRALITY_SRCS
	astrality.cpp
	cmwrapper.cpp
	protowrapper.cpp
	accountregfirstpage.cpp
	accountwrapper.cpp
	entrywrapper.cpp
	msgwrapper.cpp
	astralityutil.cpp
	vcarddialog.cpp
	)
set (ASTRALITY_FORMS
	accountregfirstpage.ui
	vcarddialog.ui
	)
set (ASTRALITY_RESOURCES azothastralityresources.qrc)
CreateTrs ("azoth_astrality" "en;es;ru_RU;uk_UA" ASTRALITY_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_astrality" "en;ru_RU" "${ASTRALITY_SRCS}" "${ASTRALITY_FORMS}" "")

QT4_WRAP_UI (ASTRALITY_UIS_H ${ASTRALITY_FORMS})
QT4_ADD_RESOURCES (ASTRALITY_RCCS ${ASTRALITY_RESOURCES})

add_library (leechcraft_azoth_astrality SHARED
	${ASTRALITY_COMPILED_TRANSLATIONS}
	${ASTRALITY_SRCS}
	${ASTRALITY_UIS_H}
	${ASTRALITY_RCCS}
	)
target_link_libraries (leechcraft_azoth_astrality
	${QT_LIBRARIES}
	${TELEPATHY_QT4_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_astrality DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${ASTRALITY_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
#install (FILES azothastralitysettings.xml DESTINATION ${LC_SETTINGS_DEST})
