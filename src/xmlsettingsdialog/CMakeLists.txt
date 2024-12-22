include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	..
	)

set (SRCS
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
	)
set (FORMS
	widgets/dataviewwidget.ui
	)

qt_wrap_ui (UIS_H ${FORMS})

foreach (SRC ${SRCS})
	set (XSD_SRCS "${XSD_SRCS};xmlsettingsdialog/${SRC}")
endforeach ()
foreach (FORM ${FORMS})
	set (XSD_FORMS "${XSD_FORMS};xmlsettingsdialog/${FORM}")
endforeach ()
set (XSD_SRCS ${XSD_SRCS} PARENT_SCOPE)
set (XSD_FORMS ${XSD_FORMS} PARENT_SCOPE)

add_util_library (xsd
	SOURCES ${SRCS} ${UIS_H}
	DEPENDS leechcraft-util-sys
	USES Network Sql Widgets Xml
	)
