cmake_minimum_required (VERSION 3.8)
project (leechcraft_azoth_sarin)
include (InitLCPlugin NO_POLICY_SCOPE)

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

include_directories (${TOX_INCLUDE_DIRS})

set (EXTRA_QT)
if (ENABLE_MEDIACALLS)
	set (EXTRA_QT Multimedia)
endif ()

LC_DEFINE_PLUGIN (azoth_sarin
	SRCS
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
	FORMS
		accregisterdetailspage.ui
		accountconfigdialog.ui
		showtoxiddialog.ui
	QT_COMPONENTS Sql Widgets ${EXTRA_QT}
	LINK_LIBRARIES PkgConfig::TOX ${EXTRA_LIBRARIES}
	)
