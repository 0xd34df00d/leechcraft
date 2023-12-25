cmake_minimum_required (VERSION 3.10)
project (azoth_acetamide)
include (InitLCPlugin NO_POLICY_SCOPE)

LC_DEFINE_PLUGIN (
	SRCS
		acetamide.cpp
		bookmarkeditwidget.cpp
		channelclentry.cpp
		channelconfigwidget.cpp
		channelhandler.cpp
		channelparticipantentry.cpp
		channelpublicmessage.cpp
		channelslistdialog.cpp
		channelslistfilterproxymodel.cpp
		channelsmanager.cpp
		clientconnection.cpp
		entrybase.cpp
		ircaccount.cpp
		ircaccountconfigurationdialog.cpp
		ircaccountconfigurationwidget.cpp
		ircerrorhandler.cpp
		ircjoingroupchat.cpp
		ircmessage.cpp
		ircparser.cpp
		ircparticipantentry.cpp
		ircprotocol.cpp
		ircserversocket.cpp
		ircserverclentry.cpp
		ircserverhandler.cpp
		invitechannelsdialog.cpp
		localtypes.cpp
		newnickservidentifydialog.cpp
		nickservidentifymanager.cpp
		nickservidentifywidget.cpp
		rplisupportparser.cpp
		parsers.cpp
		servercommandmessage.cpp
		serverinfowidget.cpp
		serverparticipantentry.cpp
		serverresponsemanager.cpp
		usercommandmanager.cpp
		vcarddialog.cpp
	SETTINGS azothacetamidesettings.xml
	QT_COMPONENTS Network Widgets Xml
	INSTALL_DESKTOP
	INSTALL_SHARE
	)

option (ENABLE_AZOTH_ACETAMIDE_TESTS "Build tests for Azoth Acetamide" ON)
if (ENABLE_AZOTH_ACETAMIDE_TESTS)
	function (AddAzothAcetamideTest _execName _cppFile _testName)
		set (_fullExecName lc_azoth_acetamide_${_execName}_test)
		add_executable (${_fullExecName} WIN32 ${_cppFile})
		target_link_libraries (${_fullExecName} ${LEECHCRAFT_LIBRARIES})
		add_test (${_testName} ${_fullExecName})
		FindQtLibs (${_fullExecName} Test)
	endfunction ()

	AddAzothAcetamideTest (urldecodertest tests/urldecodertest.cpp AzothAcetamideUrlDecoderTest)
endif ()
