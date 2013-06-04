cmake_minimum_required (VERSION 2.8)
project (leechcraft_dbusmanager)
include (InitLCPlugin OPTIONAL)

if (NOT (QT_QTDBUS_FOUND AND UNIX))
	message (ERROR "DBusManager could not be built")
endif (NOT (QT_QTDBUS_FOUND AND UNIX))

set (QT_USE_QTDBUS TRUE)
include (${QT_USE_FILE})
include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (SRCS
	dbusmanager.cpp
	core.cpp
	iinfoadaptor.cpp
	xmlsettingsmanager.cpp
	general.cpp
	generaladaptor.cpp
	tasks.cpp
	tasksadaptor.cpp
	webfilestorageadaptor.cpp
	)
set (HEADERS
	dbusmanager.h
	core.h
	iinfoadaptor.h
	xmlsettingsmanager.h
	general.h
	generaladaptor.h
	tasks.h
	tasksadaptor.h
	webfilestorageadaptor.h
	)
set (FORMS
	)
set (RESOURCES
	dbusmanagerresources.qrc
	)
CreateTrs ("dbusmanager" "ar;de;en;eo;es;fr;it;pl;ru_RU;uk_UA" COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
endif (NOT LC_NO_MOC)
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_dbusmanager SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_dbusmanager
		${QT_LIBRARIES}
		${LEECHCRAFT_LIBRARIES}
		)
install (TARGETS leechcraft_dbusmanager DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
install (FILES dbusmanagersettings.xml DESTINATION ${LC_SETTINGS_DEST})
