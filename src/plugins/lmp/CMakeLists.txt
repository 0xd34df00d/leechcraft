INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
					 ${QT_QTCORE_INCLUDE_DIR}
					 ${QT_QTGUI_INCLUDE_DIR}
					 ${QT_PHONON_INCLUDE_DIR}
					 ${CMAKE_CURRENT_BINARY_DIR}
					 ${Boost_INCLUDE_DIR}
					 ../..
					 .
					 zlib/
					 ${RBTorrent_INCLUDE_DIR})
SET (SRCS
	lmp.cpp
	core.cpp
	xmlsettingsmanager.cpp
	keyinterceptor.cpp
	)
SET (HEADERS
	lmp.h
	core.h
	xmlsettingsmanager.h
	keyinterceptor.h
	)
SET (FORMS
    tabwidget.ui
	)
SET (RESOURCES resources.qrc)
SET (TRANSLATIONS
	leechcraft_lmp_ru_RU.qm
	leechcraft_lmp_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_TARGET (lmp_translations
	COMMAND "lrelease" "leechcraft_lmp_ru_RU.ts" "leechcraft_lmp_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)

ADD_LIBRARY (leechcraft_lmp SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)

ADD_DEPENDENCIES (leechcraft_lmp lmp_translations)

TARGET_LINK_LIBRARIES (leechcraft_lmp
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)

INSTALL (TARGETS leechcraft_lmp DESTINATION ${LC_PLUGINS_DEST})
