cmake_minimum_required (VERSION 3.10)
project (otlozhu)
include (InitLCPlugin NO_POLICY_SCOPE)

option (ENABLE_OTLOZHU_SYNC "Enable synchronization support for Otlozhu" ON)

if (ENABLE_OTLOZHU_SYNC)
	add_definitions (-DENABLE_SYNC)
endif ()

LC_DEFINE_PLUGIN (
	SRCS
		otlozhu.cpp
		todotab.cpp
		core.cpp
		todomanager.cpp
		todoitem.cpp
		todostorage.cpp
		storagemodel.cpp
		addtododialog.cpp
		todolistdelegate.cpp
		todosfproxymodel.cpp
		notificationsmanager.cpp
		icalgenerator.cpp
		icalparser.cpp
		itemsmergedialog.cpp
		editcommentdialog.cpp
		editdatedialog.cpp
		xmlsettingsmanager.cpp
		$<$<BOOL:${ENABLE_OTLOZHU_SYNC}>:
			syncproxy.cpp
			stager.cpp
			stagerhandler.cpp
			>
	SETTINGS otlozhusettings.xml
	QT_COMPONENTS Widgets Xml
	LINK_LIBRARIES $<$<BOOL:${ENABLE_OTLOZHU_SYNC}>:laretz_ops>
	INSTALL_SHARE
	)
