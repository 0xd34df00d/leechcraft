cmake_minimum_required (VERSION 2.8)
project (leechcraft_snails)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set (QT_USE_QTNETWORK TRUE)
set (QT_USE_QTSQL TRUE)
set (QT_USE_QTWEBKIT TRUE)

include (${QT_USE_FILE})
include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	)
set (SRCS
	snails.cpp
	core.cpp
	mailtab.cpp
	xmlsettingsmanager.cpp
	accountslistwidget.cpp
	account.cpp
	accountconfigdialog.cpp
	message.cpp
	accountthread.cpp
	accountthreadworker.cpp
	progresslistener.cpp
	storage.cpp
	progressmanager.cpp
	mailtreedelegate.cpp
	composemessagetab.cpp
	accountfoldermanager.cpp
	attdescr.cpp
	vmimeconversions.cpp
	outputiodevadapter.cpp
	common.cpp
	mailmodel.cpp
	taskqueuemanager.cpp
	valuedmetaargument.cpp
	messagechangelistener.cpp
	foldersmodel.cpp
	folder.cpp
	viewcolumnsmanager.cpp
	texteditoradaptor.cpp
	concurrentexceptions.cpp
	mailsortmodel.cpp
	)
set (FORMS
	mailtab.ui
	accountslistwidget.ui
	accountconfigdialog.ui
	composemessagetab.ui
	)
set (RESOURCES snailsresources.qrc)
CreateTrs ("snails" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("snails" "en;ru_RU" "${SRCS}" "${FORMS}" "snailssettings.xml")
QT4_WRAP_UI (UIS_H ${FORMS})
QT4_ADD_RESOURCES (RCCS ${RESOURCES})

add_library (leechcraft_snails SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_snails
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	vmime
	)
install (TARGETS leechcraft_snails DESTINATION ${LC_PLUGINS_DEST})
install (FILES snailssettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/snails DESTINATION ${LC_SHARE_DEST})
