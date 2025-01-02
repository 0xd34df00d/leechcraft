set (XSD_SRCS
	addressesmodelmanager.cpp
	fontchooserwidget.cpp
	massfontchangedialog.cpp
	util.cpp
	wkfontswidget.cpp
	)

add_util_library (util-xsd
	SOURCES ${XSD_SRCS}
	DEPENDS
		leechcraft-xsd
		leechcraft-util-network
	USES Network Widgets
	)
