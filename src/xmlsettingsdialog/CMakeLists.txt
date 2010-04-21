SET (QT_USE_QTSCRIPT TRUE)
SET (QT_USE_QTXML TRUE)
INCLUDE (${QT_USE_FILE})
INCLUDE_DIRECTORIES (${Boost_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
	..
	)

SET (SRCS
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
	itemhandlers/itemhandlerspinbox.cpp
	itemhandlers/itemhandlerspinboxdouble.cpp
	itemhandlers/itemhandlernovalue.cpp
	itemhandlers/itemhandlerpushbutton.cpp
	itemhandlers/itemhandlercustomwidget.cpp
	)
SET (HEADERS
    xmlsettingsdialog.h
	rangewidget.h
	filepicker.h
	radiogroup.h
	scripter.h
	typeregister.h
	dir.h
	file.h
	bytearray.h
	settings.h
	basesettingsmanager.h
	fontpicker.h
	colorpicker.h
	itemhandlerfactory.h
	itemhandlers/itemhandlerbase.h
	itemhandlers/itemhandlerspinboxrange.h
	itemhandlers/itemhandleroptionssetvalue.h
	itemhandlers/itemhandlerstringsetvalue.h
	itemhandlers/itemhandlerbooleangetvalue.h
	itemhandlers/itemhandlerstringgetvalue.h
	itemhandlers/itemhandlerpath.h
	itemhandlers/itemhandlerfont.h
	itemhandlers/itemhandlercolor.h
	itemhandlers/itemhandlercheckbox.h
	itemhandlers/itemhandlergroupbox.h
	itemhandlers/itemhandlerradio.h
	itemhandlers/itemhandlercombobox.h
	itemhandlers/itemhandlerlineedit.h
	itemhandlers/itemhandlerspinboxbase.h
	itemhandlers/itemhandlerspinbox.h
	itemhandlers/itemhandlerspinboxdouble.h
	itemhandlers/itemhandlernovalue.h
	itemhandlers/itemhandlerpushbutton.h
	itemhandlers/itemhandlercustomwidget.h
	)
QT4_WRAP_CPP (MOC_SRCS ${HEADERS})
ADD_LIBRARY (xmlsettingsdialog SHARED ${SRCS} ${MOC_SRCS})
TARGET_LINK_LIBRARIES (xmlsettingsdialog
	${QT_LIBRARIES}
	)
SET_PROPERTY (TARGET xmlsettingsdialog PROPERTY SOVERSION 0.3.0)
INSTALL (TARGETS xmlsettingsdialog DESTINATION ${LIBDIR})
