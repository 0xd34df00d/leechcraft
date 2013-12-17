cmake_minimum_required (VERSION 2.8)
project (leechcraft_gmailnotifier)
include (InitLCPlugin OPTIONAL)

set(QT_USE_QTDECLARATIVE TRUE)
set(QT_USE_QTNETWORK TRUE)
set(QT_USE_QTXML TRUE)

include(${QT_USE_FILE})
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
CreateTrs("gmailnotifier" "de;en;es;fr;it;ru_RU;uk_UA" COMPILED_TRANSLATIONS)
CreateTrsUpTarget("gmailnotifier" "en;ru_RU" "${SRCS}" "" "gmailnotifiersettings.xml")
QT4_ADD_RESOURCES(RCCS ${RESOURCES})

add_library(leechcraft_gmailnotifier SHARED
	${COMPILED_TRANSLATIONS}
	${SRCS}
	${RCCS}
	${MOC_SRCS}
)
target_link_libraries(leechcraft_gmailnotifier
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
)
install(TARGETS leechcraft_gmailnotifier DESTINATION ${LC_PLUGINS_DEST})
install(FILES gmailnotifiersettings.xml DESTINATION ${LC_SETTINGS_DEST})
install(DIRECTORY share/qml/gmailnotifier DESTINATION ${LC_QML_DEST})
