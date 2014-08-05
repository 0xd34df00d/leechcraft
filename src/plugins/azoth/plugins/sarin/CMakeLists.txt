cmake_minimum_required (VERSION 2.8)
project (leechcraft_azoth_sarin)
include (InitLCPlugin OPTIONAL)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y")

include (${QT_USE_FILE})
include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (SARIN_SRCS
	sarin.cpp
	toxprotocol.cpp
	toxaccount.cpp
	toxthread.cpp
	toxcontact.cpp
	accregisterdetailspage.cpp
	showtoxiddialog.cpp
	threadexceptions.cpp
	chatmessage.cpp
	messagesmanager.cpp
	)
set (SARIN_FORMS
	accregisterdetailspage.ui
	showtoxiddialog.ui
	)
#CreateTrs ("azoth_sarin" "en;ru_RU" SARIN_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_sarin" "en;ru_RU" "${SARIN_SRCS}" "" "")
#set (SARIN_RESOURCES
#	sarinresources.qrc
#	)
#QT4_ADD_RESOURCES (SARIN_RCCS ${SARIN_RESOURCES})
QT4_WRAP_UI (SARIN_UIS_H ${SARIN_FORMS})

add_library (leechcraft_azoth_sarin SHARED
	${SARIN_COMPILED_TRANSLATIONS}
	${SARIN_SRCS}
	${SARIN_RCCS}
	${SARIN_UIS_H}
	)
target_link_libraries (leechcraft_azoth_sarin
	${QT_LIBRARIES}
	${LEECHCRAFT_LIBRARIES}
	sodium					# TODO write FindSodium.cmake
	toxcore					# TODO write FindTox.cmake
	)
install (TARGETS leechcraft_azoth_sarin DESTINATION ${LC_PLUGINS_DEST})
#install (FILES azothsarinsettings.xml DESTINATION ${LC_SETTINGS_DEST})
