if (NOT USE_QT5)
	set (QT_USE_QTSCRIPT TRUE)
	set (QT_USE_QTXML TRUE)
	include (${QT_USE_FILE})
endif ()
include_directories (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	..
	)

set (SRCS
    xmlsettingsdialog.cpp
	rangewidget.cpp
	filepicker.cpp
	radiogroup.cpp
	scripter.cpp
	typeregister.cpp
	dir.cpp
	file.cpp
	bytearray.cpp
	settings.cpp
	basesettingsmanager.cpp
	fontpicker.cpp
	colorpicker.cpp
	settingsthreadmanager.cpp
	settingsthread.cpp
	itemhandlerfactory.cpp
	itemhandlers/itemhandlerbase.cpp
	itemhandlers/itemhandlerspinboxrange.cpp
	itemhandlers/itemhandleroptionssetvalue.cpp
	itemhandlers/itemhandlerstringsetvalue.cpp
	itemhandlers/itemhandlerbooleangetvalue.cpp
	itemhandlers/itemhandlerstringgetvalue.cpp
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
	itemhandlers/itemhandlerspinboxdouble.cpp
	itemhandlers/itemhandlernovalue.cpp
	itemhandlers/itemhandlerpushbutton.cpp
	itemhandlers/itemhandlercustomwidget.cpp
	itemhandlers/itemhandlerdataview.cpp
	itemhandlers/itemhandlerlistview.cpp
	itemhandlers/itemhandlertreeview.cpp
	widgets/dataviewwidget.cpp
	)
set (FORMS
	widgets/dataviewwidget.ui
	)

if (USE_QT5)
	QT5_WRAP_UI (UIS_H ${FORMS})
else ()
	QT4_WRAP_UI (UIS_H ${FORMS})
endif ()

foreach (SRC ${SRCS})
	set (XSD_SRCS "${XSD_SRCS};xmlsettingsdialog/${SRC}")
endforeach ()
foreach (FORM ${FORMS})
	set (XSD_FORMS "${XSD_FORMS};xmlsettingsdialog/${FORM}")
endforeach ()
set (XSD_SRCS ${XSD_SRCS} PARENT_SCOPE)
set (XSD_FORMS ${XSD_FORMS} PARENT_SCOPE)

add_library (leechcraft-xsd SHARED
	${SRCS}
	${UIS_H})
target_link_libraries (leechcraft-xsd
	${QT_LIBRARIES}
	leechcraft-util-sys
	)
set_property (TARGET leechcraft-xsd PROPERTY SOVERSION 0.3.0)
install (TARGETS leechcraft-xsd DESTINATION ${LIBDIR})

if (USE_QT5)
	QT5_USE_MODULES (leechcraft-xsd Xml Sql Widgets Script Network)
endif ()
