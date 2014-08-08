set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package (Wt REQUIRED)
add_definitions (-DQT_NO_KEYWORDS -DWT_NO_SLOT_MACROS)

include_directories (${AGGREGATOR_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${Wt_INCLUDE_DIR}
	)
set (WEBACCESS_SRCS
	webaccess.cpp
	servermanager.cpp
	aggregatorapp.cpp
	readchannelsfilter.cpp
	readitemsfilter.cpp
	q2wproxymodel.cpp
	util.cpp
	serverupdater.cpp
	xmlsettingsmanager.cpp
	)

CreateTrs ("aggregator_webaccess" "en;ru_RU" WEBACCESS_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("aggregator_webaccess" "en;ru_RU" "${WEBACCESS_SRCS}" "" "aggregatorwebaccesssettings.xml")

add_library (leechcraft_aggregator_webaccess SHARED
	${WEBACCESS_COMPILED_TRANSLATIONS}
	${WEBACCESS_SRCS}
	${WEBACCESS_MOC_SRCS}
	)
target_link_libraries (leechcraft_aggregator_webaccess
	${LEECHCRAFT_LIBRARIES}
	${Wt_LIBRARY}
	${Wt_HTTP_LIBRARY}
	leechcraft_aggregator_util
	)
install (TARGETS leechcraft_aggregator_webaccess DESTINATION ${LC_PLUGINS_DEST})
install (FILES aggregatorwebaccesssettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_aggregator_webaccess Core)
