cmake_minimum_required (VERSION 3.10)
project (blogique)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		accountslistwidget.cpp
		accountsselectdialog.cpp
		addaccountwizardfirstpage.cpp
		addcommentdialog.cpp
		blogique.cpp
		blogentrieswidget.cpp
		blogiquewidget.cpp
		calendarwidget.cpp
		commentsmanager.cpp
		commentsmodel.cpp
		commentswidget.cpp
		core.cpp
		draftentrieswidget.cpp
		exportwizard.cpp
		pluginproxy.cpp
		profiledialog.cpp
		selecttargetdelegate.cpp
		sortcommentsproxymodel.cpp
		storagemanager.cpp
		submittodialog.cpp
		tagsproxymodel.cpp
		updateentriesdialog.cpp
		utils.cpp
	SETTINGS blogiquesettings.xml
	QT_COMPONENTS PrintSupport QuickWidgets Sql Xml
	INSTALL_SHARE
	)

SUBPLUGIN (METIDA "Build Metida, LiveJournal blogging platform")
SUBPLUGIN (HESTIA "Build Hestia, local blogging platform")
