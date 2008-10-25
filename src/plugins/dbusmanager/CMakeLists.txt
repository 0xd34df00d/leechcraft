INCLUDE_DIRECTORIES (${QT_INCLUDE_DIR}
					 ${QT_QTCORE_INCLUDE_DIR}
					 ${QT_QTGUI_INCLUDE_DIR}
					 ${CMAKE_CURRENT_BINARY_DIR}
					 ${Boost_INCLUDE_DIR}
					 ../..
					 .
					 zlib/)

SET (SRCS
	dbusmanager.cpp
	)

SET (HEADERS
	dbusmanager.h
	)

SET (FORMS
	)

SET (RESOURCES
	)

SET (TRANSLATIONS
	leechcraft_dbusmanager_ru_RU.qm
	leechcraft_dbusmanager_uk_UA.qm
	)

QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

ADD_CUSTOM_TARGET (dbusmanager_translations
	COMMAND "lrelease" "leechcraft_dbusmanager_ru_RU.ts" "leechcraft_dbusmanager_uk_UA.ts"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_dbusmanager SHARED
	${SRCS}
	${MOC_SRCS}
	${UIS_H}
	${RCCS}
	)
ADD_DEPENDENCIES (leechcraft_dbusmanager dbusmanager_translations)
TARGET_LINK_LIBRARIES (leechcraft_dbusmanager
		${QT_LIBRARIES}
		plugininterface
		xmlsettingsdialog
		)
INSTALL (TARGETS leechcraft_dbusmanager DESTINATION ${LC_PLUGINS_DEST})
