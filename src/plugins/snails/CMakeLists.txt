cmake_minimum_required (VERSION 3.3)
project (leechcraft_snails)
include (InitLCPlugin)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package (VMime REQUIRED)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${VMIME_INCLUDE_DIR}
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
	messagechangelistener.cpp
	foldersmodel.cpp
	folder.cpp
	viewcolumnsmanager.cpp
	texteditoradaptor.cpp
	mailsortmodel.cpp
	headersviewwidget.cpp
	mailwebpage.cpp
	mailmodelsmanager.cpp
	accountdatabase.cpp
	messagelistactioninfo.cpp
	messagelisteditormanager.cpp
	messagelistactionsmanager.cpp
	mailtabreadmarker.cpp
	composemessagetabfactory.cpp
	accountsmanager.cpp
	accountlogger.cpp
	tracerfactory.cpp
	tracer.cpp
	msgtemplatesmanager.cpp
	templateseditorwidget.cpp
	multieditorwidget.cpp
	templatesstorage.cpp
	templatepattern.cpp
	structures.cpp
	threadpool.cpp
	util.cpp
	attachmentsfetcher.cpp
	accountthreadnotifier.cpp
	certificateverifier.cpp
	)
set (FORMS
	mailtab.ui
	accountslistwidget.ui
	accountconfigdialog.ui
	composemessagetab.ui
	headersviewwidget.ui
	templateseditorwidget.ui
	multieditorwidget.ui
	)
set (RESOURCES snailsresources.qrc)
CreateTrs ("snails" "en;ru_RU" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("snails" "en;ru_RU" "${SRCS}" "${FORMS}" "snailssettings.xml")
QtWrapUi (UIS_H ${FORMS})
QtAddResources (RCCS ${RESOURCES})

add_library (leechcraft_snails SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${UIS_H}
	${RCCS}
	)
target_link_libraries (leechcraft_snails
	${LEECHCRAFT_LIBRARIES}
	${VMIME_LIBRARIES}
	)
install (TARGETS leechcraft_snails DESTINATION ${LC_PLUGINS_DEST})
install (FILES snailssettings.xml DESTINATION ${LC_SETTINGS_DEST})
install (DIRECTORY share/snails DESTINATION ${LC_SHARE_DEST})

FindQtLibs (leechcraft_snails Concurrent Network Sql WebKitWidgets)
