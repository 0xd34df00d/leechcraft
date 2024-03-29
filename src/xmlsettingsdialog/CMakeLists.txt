include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	..
	)

set (SRCS
    xmlsettingsdialog.cpp
	scripter.cpp
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

QT5_WRAP_UI (UIS_H ${FORMS})

foreach (SRC ${SRCS})
	set (XSD_SRCS "${XSD_SRCS};xmlsettingsdialog/${SRC}")
endforeach ()
foreach (FORM ${FORMS})
	set (XSD_FORMS "${XSD_FORMS};xmlsettingsdialog/${FORM}")
endforeach ()
set (XSD_SRCS ${XSD_SRCS} PARENT_SCOPE)
set (XSD_FORMS ${XSD_FORMS} PARENT_SCOPE)

find_package (Qt5 COMPONENTS Widgets Script Network Sql Xml)

add_library (leechcraft-xsd${LC_LIBSUFFIX} SHARED
	${SRCS}
	${UIS_H})
target_link_libraries (leechcraft-xsd${LC_LIBSUFFIX}
	${QT_LIBRARIES}
	leechcraft-util-sys${LC_LIBSUFFIX}
	Qt5::Xml
	Qt5::Sql
	Qt5::Widgets
	Qt5::Script
	Qt5::Network
	)

set_property (TARGET leechcraft-xsd${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.2)
install (TARGETS leechcraft-xsd${LC_LIBSUFFIX} DESTINATION ${LIBDIR})
