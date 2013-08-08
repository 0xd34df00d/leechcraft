CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_auscrie)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	auscrie.cpp
	shooterdialog.cpp
	poster.cpp
	requestbuilder.cpp
	)
SET (FORMS
	shooterdialog.ui
	)
SET (RESOURCES auscrieresources.qrc)
CreateTrs ("auscrie" "de;en;es;fr;it;oc;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

ADD_LIBRARY (leechcraft_auscrie SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)

FindQtLibs (leechcraft_auscrie Network)

TARGET_LINK_LIBRARIES (leechcraft_auscrie
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_auscrie DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES ${COMPILED_TRANSLATIONS} DESTINATION ${LC_TRANSLATIONS_DEST})
