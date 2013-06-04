cmake_minimum_required (VERSION 2.8)
project (leechcraft_netstoremanager_yandexdisk)
include (InitLCPlugin OPTIONAL)

include_directories (${NETSTOREMANAGER_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)

set (YANDEXDISK_SRCS
	yandexdisk.cpp
	account.cpp
	uploadmanager.cpp
	authmanager.cpp
	urls.cpp
	flgetter.cpp
	flitem.cpp
	common.cpp
	simpleactor.cpp
	actorbase.cpp
	)
set (YANDEXDISK_HEADERS
	yandexdisk.h
	account.h
	uploadmanager.h
	authmanager.h
	urls.h
	flgetter.h
	flitem.h
	common.h
	simpleactor.h
	actorbase.h
	)
set (YANDEXDISK_FORMS
	)
set (YANDEXDISK_RESOURCES netstoremanageryandexdiskresources.qrc)
CreateTrs ("netstoremanager_yandexdisk" "en;es;ru_RU;uk_UA" YANDEXDISK_COMPILED_TRANSLATIONS)

if (NOT LC_NO_MOC)
	QT4_WRAP_CPP (YANDEXDISK_MOC_SRCS ${YANDEXDISK_HEADERS})
endif (NOT LC_NO_MOC)
#QT4_WRAP_UI (YANDEXDISK_UIS_H ${YANDEXDISK_FORMS})
QT4_ADD_RESOURCES (YANDEXDISK_RCCS ${YANDEXDISK_RESOURCES})

add_library (leechcraft_netstoremanager_yandexdisk SHARED
	${YANDEXDISK_COMPILED_TRANSLATIONS}
	${YANDEXDISK_SRCS}
	${YANDEXDISK_MOC_SRCS}
	${YANDEXDISK_UIS_H}
	${YANDEXDISK_RCCS}
	)
target_link_libraries (leechcraft_netstoremanager_yandexdisk
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_netstoremanager_yandexdisk DESTINATION ${LC_PLUGINS_DEST})
install (FILES ${YANDEXDISK_COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
#install (FILES netstoremanageryandexdisksettings.xml DESTINATION ${LC_SETTINGS_DEST})
