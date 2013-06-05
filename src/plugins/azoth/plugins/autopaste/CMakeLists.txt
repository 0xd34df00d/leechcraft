cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_autopaste)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTNETWORK TRUE)
include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (AUTOPASTE_SRCS
	autopaste.cpp
	bpasteservice.cpp
	codepadservice.cpp
	pastedialog.cpp
	pasteservicebase.cpp
	pasteservicefactory.cpp
	xmlsettingsmanager.cpp
	)
set (AUTOPASTE_HEADERS
	autopaste.h
	bpasteservice.h
	codepadservice.h
	pastedialog.h
	pasteservicebase.h
	pasteservicefactory.h
	xmlsettingsmanager.h
	)
set (AUTOPASTE_FORMS
	pastedialog.ui
	)
set (AUTOPASTE_RESOURCES azothautopasteresources.qrc)
CreateTrs ("azoth_autopaste" "en;es;ru_RU;uk_UA" AUTOPASTE_COMPILED_TRANSLATIONS)
if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (AUTOPASTE_MOC_SRCS ${AUTOPASTE_HEADERS})
endif ()
QT4_WRAP_UI (AUTOPASTE_UIS_H ${AUTOPASTE_FORMS})
QT4_ADD_RESOURCES (AUTOPASTE_RCCS ${AUTOPASTE_RESOURCES})

add_library (leechcraft_azoth_autopaste SHARED
	${AUTOPASTE_COMPILED_TRANSLATIONS}
	${AUTOPASTE_SRCS}
	${AUTOPASTE_MOC_SRCS}
	${AUTOPASTE_RCCS}
	${AUTOPASTE_UIS_H}
	)
target_link_libraries (leechcraft_azoth_autopaste
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_autopaste DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothautopastesettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (FILES ${AUTOPASTE_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
