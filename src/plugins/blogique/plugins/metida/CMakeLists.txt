cmake_minimum_required (VERSION 3.10)
project (leechcraft_blogique_metida)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		addeditentrydialog.cpp
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
	RESOURCES blogiquemetidaresources.qrc
	SETTINGS blogiquemetidasettings.xml
	QT_COMPONENTS Network Quick Sql Widgets Xml XmlPatterns
	)
