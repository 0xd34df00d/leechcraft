cmake_minimum_required (VERSION 3.3)
project (leechcraft_snails)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package (PkgConfig)
pkg_check_modules (Vmime REQUIRED IMPORTED_TARGET vmime)

LC_DEFINE_PLUGIN (
	SRCS
		snails.cpp
		core.cpp
		mailtab.cpp
		xmlsettingsmanager.cpp
		accountslistwidget.cpp
		account.cpp
		accountconfigdialog.cpp
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
		tracebytecounter.cpp
		address.cpp
		outgoingmessage.cpp
		messageinfo.cpp
		messagebodies.cpp
		accountconfig.cpp
		accountaddwizard.cpp
		mailwebpagenam.cpp
	SETTINGS snailssettings.xml
	QT_COMPONENTS Concurrent Network Sql WebKitWidgets
	LINK_LIBRARIES PkgConfig::Vmime
	INSTALL_SHARE
	)
