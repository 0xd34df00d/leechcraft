cmake_minimum_required (VERSION 2.8)
project (leechcraft_blogique_hestia)
include (InitLCPlugin OPTIONAL)

set (QT_USE_QTSQL TRUE)
include (${QT_USE_FILE})

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${BLOGIQUE_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/blogique
	)

set (HESTIA_SRCS
	accountconfigurationdialog.cpp
	accountconfigurationwidget.cpp
	accountstorage.cpp
	core.cpp
	hestia.cpp
	localblogaccount.cpp
	localbloggingplatform.cpp
	xmlsettingsmanager.cpp
	)

set (HESTIA_HEADERS
	accountconfigurationdialog.h
	accountstorage.h
	core.h
	hestia.h
	accountconfigurationwidget.h
	localblogaccount.h
	localbloggingplatform.h
	xmlsettingsmanager.h
	)

set (HESTIA_FORMS
	accountconfigurationdialog.ui
	accountconfigurationwidget.ui
 	)

# set (HESTIA_RESOURCES blogiquehestiaresources.qrc)

# CreateTrVars ("blogique_hestia" "de;en;es;fr;it;ru_RU;uk_UA" HESTIA_TRANSLATIONS HESTIA_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (HESTIA_MOC_SRCS ${HESTIA_HEADERS})
endif (NOT LC_NO_MOC)
QT4_WRAP_UI (HESTIA_UIS_H ${HESTIA_FORMS})
QT4_ADD_RESOURCES (HESTIA_RCCS ${HESTIA_RESOURCES})

# add_custom_command (OUTPUT
# 	${HESTIA_COMPILED_TRANSLATIONS}
# 	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${HESTIA_TRANSLATIONS}
# 	DEPENDS ${HESTIA_TRANSLATIONS}
# 	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# 	)

add_library (leechcraft_blogique_hestia SHARED
	${HESTIA_COMPILED_TRANSLATIONS}
	${HESTIA_SRCS}
	${HESTIA_MOC_SRCS}
	${HESTIA_UIS_H}
	${HESTIA_RCCS}
	)

target_link_libraries (leechcraft_blogique_hestia
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blogique_hestia
		DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${HESTIA_COMPILED_TRANSLATIONS}
		DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES blogiquehestiasettings.xml
		DESTINATION ${LC_SETTINGS_DEST})