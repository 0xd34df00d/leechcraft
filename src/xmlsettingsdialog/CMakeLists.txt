include_directories (../)

add_util_library (xsd
	SOURCES
		xmlsettingsdialog.cpp
		settings.cpp
		basesettingsmanager.cpp
		settingsthreadmanager.cpp
		settingsthread.cpp
		itemhandlerfactory.cpp
		itemhandlers/defaultvaluegetters.cpp
		itemhandlers/itemhandlerbase.cpp
		itemhandlers/itemhandlerspinboxrange.cpp
		itemhandlers/itemhandlerpath.cpp
		itemhandlers/itemhandlerfont.cpp
		itemhandlers/itemhandlercolor.cpp
		itemhandlers/itemhandlercheckbox.cpp
		itemhandlers/itemhandlergroupbox.cpp
		itemhandlers/itemhandlerradio.cpp
		itemhandlers/itemhandlercombobox.cpp
		itemhandlers/itemhandlerlineedit.cpp
		itemhandlers/itemhandlermultiline.cpp
		itemhandlers/itemhandlerspinbox.cpp
		itemhandlers/itemhandlerpushbutton.cpp
		itemhandlers/itemhandlercustomwidget.cpp
		itemhandlers/itemhandlerdataview.cpp
		widgets/colorpicker.cpp
		widgets/dataviewwidget.cpp
		widgets/filepicker.cpp
		widgets/fontpicker.cpp
		widgets/radiogroup.cpp
		widgets/rangewidget.cpp
	DEPENDS leechcraft-util-sys
	USES Network Sql Widgets Xml
	)
