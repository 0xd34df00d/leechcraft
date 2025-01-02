set (XSD_SRCS
	addressesmodelmanager.cpp
	fontchooserwidget.cpp
	massfontchangedialog.cpp
	util.cpp
	wkfontswidget.cpp
	)
set (XSD_FORMS
	fontchooserwidget.ui
	massfontchangedialog.ui
	wkfontswidget.ui
	)

qt_wrap_ui (XSD_UIS_H ${XSD_FORMS})

add_util_library (util-xsd
	SOURCES ${XSD_SRCS} ${XSD_UIS_H}
	DEPENDS
		leechcraft-xsd
		leechcraft-util-network
	USES Network Widgets
	)
