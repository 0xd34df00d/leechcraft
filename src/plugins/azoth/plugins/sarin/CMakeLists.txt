cmake_minimum_required (VERSION 3.10)
project (leechcraft_azoth_sarin)
include (InitLCPlugin NO_POLICY_SCOPE)

find_package(PkgConfig)
pkg_check_modules(TOX REQUIRED IMPORTED_TARGET toxcore)

add_compile_definitions (
	$<$<BOOL:${ENABLE_MEDIACALLS}>:ENABLE_MEDIACALLS>
	)

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
		filetransfermanager.cpp
		filetransferbase.cpp
		filetransferin.cpp
		filetransferout.cpp
		$<$<BOOL:${ENABLE_MEDIACALLS}>:
			audiocall.cpp
			audiocalldevice.cpp
			callmanager.cpp
			calldatawriter.cpp
			>
	QT_COMPONENTS Sql Widgets ${EXTRA_MULTIMEDIA_LIBRARY}
	LINK_LIBRARIES PkgConfig::TOX
	)
