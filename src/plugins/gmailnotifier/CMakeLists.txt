cmake_minimum_required (VERSION 3.3)
project (leechcraft_gmailnotifier)
include (InitLCPlugin OPTIONAL)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${Boost_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
)
set(SRCS
	gmailnotifier.cpp
	xmlsettingsmanager.cpp
	gmailchecker.cpp
	convinfo.cpp
	notifier.cpp
	quarkmanager.cpp
	maillistview.cpp
)
set(RESOURCES
	res/resources.qrc
)
CreateTrs("gmailnotifier" "de;en;es;fr;it;ro;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("gmailnotifier" "en;ru_RU" "${SRCS}" "" "gmailnotifiersettings.xml")
QtAddResources(RCCS ${RESOURCES})

add_library(leechcraft_gmailnotifier SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
)
target_link_libraries(leechcraft_gmailnotifier
	${LEECHCRAFT_LIBRARIES}
)
install(TARGETS leechcraft_gmailnotifier DESTINATION ${LC_PLUGINS_DEST})
install(FILES gmailnotifiersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install(DIRECTORY share/qml5/gmailnotifier DESTINATION ${LC_QML5_DEST})

FindQtLibs (leechcraft_gmailnotifier Network QuickWidgets Xml)
