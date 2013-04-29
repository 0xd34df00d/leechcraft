CMAKE_MINIMUM_REQUIRED (VERSION 2.8)
PROJECT (leechcraft_lhtr)
INCLUDE (InitLCPlugin OPTIONAL)

SET (QT_USE_QTNETWORK TRUE)
SET (QT_USE_QTWEBKIT TRUE)
INCLUDE (${QT_USE_FILE})
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
QT4_WRAP_UI (UIS_H ${FORMS})
CreateTrs ("lhtr" "en;es;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("lhtr" "en;ru_RU" "${SRCS}" "${FORMS}" "lhtrsettings.xml")

SET (LHTR_RESOURCES lhtrresources.qrc)
QT4_ADD_RESOURCES (LHTR_RCCS ${LHTR_RESOURCES})

ADD_LIBRARY (leechcraft_lhtr SHARED
	${LHTR_RCCS}
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_lhtr
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	)
INSTALL (TARGETS leechcraft_lhtr DESTINATION ${LC_PLUGINS_DEST})
INSTALL (FILES lhtrsettings.xml DESTINATION ${LC_SETTINGS_DEST})
