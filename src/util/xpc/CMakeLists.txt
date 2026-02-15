add_util_library (util-xpc
	SRCS
		basehookinterconnector.cpp
		defaulthookproxy.cpp
		notificationactionhandler.cpp
		passutils.cpp
		stdanfields.cpp
		stddatafiltermenucreator.cpp
		util.cpp
		anutil.cpp
		introspectable.cpp
		screensaverprohibitor.cpp
		anconstantsdefs.cpp
		downloadhelpers.cpp
		downloaderrorstrings.cpp
		progressmanager.cpp
	DEPENDS
		leechcraft-util-models
		leechcraft-util-sll
		leechcraft-util-sys
	USES Widgets
	)
