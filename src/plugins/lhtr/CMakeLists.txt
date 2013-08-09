CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_lhtr)
INCLUDE (InitLCPlugin OPTIONAL)

INCLUDE_DIRECTORIES (
	${CMAKE_CURRENT_BINARY_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
SET (SRCS
	lhtr.cpp
	richeditorwidget.cpp
	hyperlinkdialog.cpp
	imagedialog.cpp
	finddialog.cpp
	inserttabledialog.cpp
	xmlsettingsmanager.cpp
	)
SET (FORMS
	richeditorwidget.ui
	hyperlinkdialog.ui
	imagedialog.ui
	finddialog.ui
	inserttabledialog.ui
	)
QtWrapUi (UIS_H ${FORMS})
CreateTrs ("lhtr" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("lhtr" "en;ru_RU" "${SRCS}" "${FORMS}" "lhtrsettings.xml")

SET (LHTR_RESOURCES lhtrresources.qrc)
QtAddResources (LHTR_RCCS ${LHTR_RESOURCES})

ADD_LIBRARY (leechcraft_lhtr SHARED
	${LHTR_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	)

FindQtLibs (leechcraft_lhtr Network WebKitWidgets)

TARGET_LINK_LIBRARIES (leechcraft_lhtr
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_lhtr DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES lhtrsettings.xml DESTINATION ${LC_SETTINGS_DEST})
