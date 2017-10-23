cmake_minimum_required (VERSION 3.3)
project (leechcraft_blogique_metida)
include (InitLCPlugin OPTIONAL)

include_directories (${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${BLOGIQUE_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/blogique
	)

set (METIDA_SRCS
	addeditentrydialog.cpp
	core.cpp
	frienditemdelegate.cpp
	friendsproxymodel.cpp
	ljaccount.cpp
	ljaccountconfigurationdialog.cpp
	ljaccountconfigurationwidget.cpp
	ljbloggingplatform.cpp
	ljfriendentry.cpp
	ljprofile.cpp
	ljxmlrpc.cpp
	localstorage.cpp
	metida.cpp
	polldialog.cpp
	postoptionswidget.cpp
	profiletypes.cpp
	profilewidget.cpp
	selectgroupsdialog.cpp
	sendmessagedialog.cpp
	updatetypedialog.cpp
	utils.cpp
	xmlsettingsmanager.cpp
	)

set (METIDA_FORMS
	addeditentrydialog.ui
	ljaccountconfigurationdialog.ui
	ljaccountconfigurationwidget.ui
	polldialog.ui
	profilewidget.ui
	postoptionswidget.ui
	selectgroupsdialog.ui
	sendmessagedialog.ui
	updatetypedialog.ui
	)

set (METIDA_RESOURCES blogiquemetidaresources.qrc metidaresources.qrc)

QtWrapUi (METIDA_UIS_H ${METIDA_FORMS})
QtAddResources (METIDA_RCCS ${METIDA_RESOURCES})

CreateTrs ("blogique_metida" "en;ru_RU" METIDA_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("blogique_metida" "en;ru_RU" "${METIDA_SRCS}" "${METIDA_FORMS}" "blogiquemetidasettings.xml")

add_library (leechcraft_blogique_metida SHARED
	${METIDA_RCCS}
	${METIDA_COMPILED_TRANSLATIONS}
	${METIDA_SRCS}
	${METIDA_UIS_H}
	${METIDA_RCCS}
	)

target_link_libraries (leechcraft_blogique_metida
	${LEECHCRAFT_LIBRARIES}
	)

install (TARGETS leechcraft_blogique_metida DESTINATION ${LC_PLUGINS_DEST})
install (FILES blogiquemetidasettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_blogique_metida Network Quick Sql Widgets Xml XmlPatterns)
