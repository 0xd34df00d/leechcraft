cmake_minimum_required (VERSION 3.8)
project (leechcraft_azoth_sarin)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package(PkgConfig)
pkg_check_modules(TOX REQUIRED IMPORTED_TARGET toxcore)

LC_DEFINE_PLUGIN (
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
	QT_COMPONENTS Sql Widgets ${EXTRA_MULTIMEDIA_LIBRARY}
	LINK_LIBRARIES PkgConfig::TOX
	)
