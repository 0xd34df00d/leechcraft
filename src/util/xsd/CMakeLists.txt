add_util_library (util-xsd
	SRCS
		addressesmodelmanager.cpp
		fontchooserwidget.cpp
		massfontchangedialog.cpp
		util.cpp
		wkfontswidget.cpp
	DEPENDS
		leechcraft-xsd
		leechcraft-util-network
	USES Network Widgets
	)
