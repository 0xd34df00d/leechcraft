INCLUDE_DIRECTORIES (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	)

SET (ACETAMIDE_SRCS
	acetamide.cpp
	core.cpp
	ircprotocol.cpp
	ircaccount.cpp
	ircaccountconfigurationdialog.cpp
	ircaddserverdialog.cpp
	ircadddefaultchannelsdialog.cpp
	irceditchanneldialog.cpp
	ircjoingroupchat.cpp
	)

SET (ACETAMIDE_HEADERS
	acetamide.h
	core.h
	ircprotocol.h
	ircaccount.h
	ircaccountconfigurationdialog.h
	ircaddserverdialog.h
	ircadddefaultchannelsdialog.h
	irceditchanneldialog.h
	ircjoingroupchat.h
	)

SET (ACETAMIDE_FORMS
	ircaccountconfigurationdialog.ui
	ircadddefaultchannelsdialog.ui
	ircaddserverdialog.ui
	irceditchanneldialog.ui
	ircjoingroupchat.ui
	)

SET (ACETAMIDE_TRANSLATIONS
	leechcraft_azoth_acetamide_ru_RU.ts
	leechcraft_azoth_acetamide_uk_UA.ts
	leechcraft_azoth_acetamide_en.ts
	)
SET (ACETAMIDE_COMPILED_TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_azoth_acetamide_ru_RU.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_azoth_acetamide_uk_UA.qm
	${CMAKE_CURRENT_SOURCE_DIR}/leechcraft_azoth_acetamide_en.qm
	)

QT4_WRAP_CPP (ACETAMIDE_MOC_SRCS ${ACETAMIDE_HEADERS})
QT4_WRAP_UI (ACETAMIDE_UIS_H ${ACETAMIDE_FORMS})

ADD_CUSTOM_COMMAND (OUTPUT
	${ACETAMIDE_COMPILED_TRANSLATIONS}
	COMMAND "${QT_LRELEASE_EXECUTABLE}" ${ACETAMIDE_TRANSLATIONS}
	DEPENDS ${ACETAMIDE_TRANSLATIONS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
ADD_LIBRARY (leechcraft_azoth_acetamide SHARED
	${ACETAMIDE_COMPILED_TRANSLATIONS}
	${ACETAMIDE_SRCS}
	${ACETAMIDE_MOC_SRCS}
	${ACETAMIDE_UIS_H}
	)
TARGET_LINK_LIBRARIES (leechcraft_azoth_acetamide
	${QT_LIBRARIES}
	plugininterface
	xmlsettingsdialog
	)
INSTALL (TARGETS leechcraft_azoth_acetamide DESTINATION ${LC_PLUGINS_DEST})
