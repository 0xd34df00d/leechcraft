cmake_minimum_required (VERSION 3.3)
project (leechcraft_azoth_chathistory)
include (InitLCPlugin)

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (CHATHISTORY_SRCS
	chathistory.cpp
	storage.cpp
	chathistorywidget.cpp
	storagethread.cpp
	historymessage.cpp
	chatfindbox.cpp
	xmlsettingsmanager.cpp
	historyvieweventfilter.cpp
	storagemanager.cpp
	loggingstatekeeper.cpp
	)
set (CHATHISTORY_FORMS
	chathistorywidget.ui
	)
set (CHATHISTORY_RESOURCES azothchathistoryresources.qrc)

CreateTrs ("azoth_chathistory" "en;es;ru_RU;uk_UA" CHATHISTORY_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_chathistory" "en;ru_RU" "${CHATHISTORY_SRCS}" "${CHATHISTORY_FORMS}" "azothchathistorysettings.xml")

QtWrapUi (CHATHISTORY_UIS_H ${CHATHISTORY_FORMS})
QtAddResources (CHATHISTORY_RCCS ${CHATHISTORY_RESOURCES})

add_library (leechcraft_azoth_chathistory SHARED
	${CHATHISTORY_COMPILED_TRANSLATIONS}
	${CHATHISTORY_SRCS}
	${CHATHISTORY_UIS_H}
	${CHATHISTORY_RCCS}
	)
target_link_libraries (leechcraft_azoth_chathistory
	${LEECHCRAFT_LIBRARIES}
	)
install (TARGETS leechcraft_azoth_chathistory DESTINATION ${LC_PLUGINS_DEST})
install (FILES azothchathistorysettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_chathistory Core Sql Widgets)
