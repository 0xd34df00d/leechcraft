cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_p100q)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTWEBKIT TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (P100Q_SRCS
	xmlsettingsmanager.cpp
	p100q.cpp
	)
set (P100Q_HEADERS
	xmlsettingsmanager.h
	p100q.h
	)
set (P100Q_RESOURCES azothp100qresources.qrc)

CreateTrs ("azoth_p100q" "en;es;ru_RU;uk_UA" P100Q_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_p100q" "en;ru_RU" "${P100Q_SRCS}" "" "azothp100qsettings.xml")
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (P100Q_MOC_SRCS ${P100Q_HEADERS})
endif (NOT LC_NO_MOC)
QT4_ADD_RESOURCES (P100Q_RCCS ${P100Q_RESOURCES})

add_library (leechcraft_azoth_p100q SHARED
	${P100Q_SRCS}
	${P100Q_MOC_SRCS}
	${P100Q_RCCS}
	${P100Q_COMPILED_TRANSLATIONS}
	)
target_link_libraries (leechcraft_azoth_p100q
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_p100q DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothp100qsettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${P100Q_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
