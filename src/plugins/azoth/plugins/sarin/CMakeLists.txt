cmake_minimum_required (VERSION 3.8)
project (leechcraft_azoth_sarin)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package(PkgConfig)
pkg_check_modules(TOX REQUIRED IMPORTED_TARGET toxcore)

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
	LINK_LIBRARIES PkgConfig::TOX
	)
