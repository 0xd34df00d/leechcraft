find_package (Wt REQUIRED)
add_definitions (-DQT_NO_KEYWORDS -DWT_NO_SLOT_MACROS)

LC_DEFINE_PLUGIN (aggregator_webaccess
	SRCS
		webaccess.cpp
		servermanager.cpp
		aggregatorapp.cpp
		readchannelsfilter.cpp
		readitemsfilter.cpp
		q2wproxymodel.cpp
		util.cpp
		serverupdater.cpp
		xmlsettingsmanager.cpp
	SETTINGS aggregatorwebaccesssettings.xml
	QT_COMPONENTS Core
	LINK_LIBRARIES
		Wt::Wt
		Wt::HTTP
		leechcraft_aggregator_util
	)
