cmake_minimum_required (VERSION 3.8)
project (leechcraft_azoth_sarin)
include (InitLCPlugin OPTIONAL)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

find_package (Tox)

option (USE_AZOTH_SARIN_NACL "Use NaCl crypto library instead of libsodium" OFF)

set (EXTRA_LIBRARIES)
if (USE_AZOTH_SARIN_NACL)
	find_package (NaCl)

	set (EXTRA_LIBRARIES ${NACL_LIBRARIES})
	include_directories (${NACL_INCLUDE_DIRS})
	add_definitions (-DUSE_NACL)
else ()
	find_package (Sodium)

	set (EXTRA_LIBRARIES ${SODIUM_LIBRARIES})
	include_directories (${SODIUM_INCLUDE_DIRS})
endif ()

include_directories (${AZOTH_INCLUDE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${TOX_INCLUDE_DIRS}
	${LEECHCRAFT_INCLUDE_DIR}
	${LEECHCRAFT_INCLUDE_DIR}/azoth
	)
set (SARIN_SRCS
	sarin.cpp
	toxprotocol.cpp
	toxaccount.cpp
	toxthread.cpp
	toxcontact.cpp
	toxlogger.cpp
	callbackmanager.cpp
	accregisterdetailspage.cpp
	showtoxiddialog.cpp
	threadexceptions.cpp
	chatmessage.cpp
	messagesmanager.cpp
	accountconfigdialog.cpp
	toxaccountconfiguration.cpp
	util.cpp
	callmanager.cpp
	audiocall.cpp
	audiocalldevice.cpp
	calldatawriter.cpp
	filetransfermanager.cpp
	filetransferbase.cpp
	filetransferin.cpp
	filetransferout.cpp
	)
set (SARIN_FORMS
	accregisterdetailspage.ui
	accountconfigdialog.ui
	showtoxiddialog.ui
	)
#CreateTrs ("azoth_sarin" "en;ru_RU" SARIN_COMPILED_TRANSLATIONS)
CreateTrsUpTarget ("azoth_sarin" "en;ru_RU" "${SARIN_SRCS}" "" "")
#set (SARIN_RESOURCES
#	sarinresources.qrc
#	)
#QT4_ADD_RESOURCES (SARIN_RCCS ${SARIN_RESOURCES})
QtWrapUi (SARIN_UIS_H ${SARIN_FORMS})

add_library (leechcraft_azoth_sarin SHARED
	${SARIN_COMPILED_TRANSLATIONS}
	${SARIN_SRCS}
	${SARIN_RCCS}
	${SARIN_UIS_H}
	)
target_link_libraries (leechcraft_azoth_sarin
	${LEECHCRAFT_LIBRARIES}
	${TOXCORE_LIBRARY}
	${TOXAV_LIBRARY}
	${EXTRA_LIBRARIES}
	)
set_property(TARGET leechcraft_azoth_sarin PROPERTY CXX_STANDARD 17)
install (TARGETS leechcraft_azoth_sarin DESTINATION ${LC_PLUGINS_DEST})
#install (FILES azothsarinsettings.xml DESTINATION ${LC_SETTINGS_DEST})

FindQtLibs (leechcraft_azoth_sarin Concurrent Widgets)

if (ENABLE_MEDIACALLS)
	FindQtLibs (leechcraft_azoth_sarin Multimedia)
endif ()
