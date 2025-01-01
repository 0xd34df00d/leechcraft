set (XPC_SRCS
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
	)

add_util_library (util-xpc
	SOURCES ${XPC_SRCS}
	DEPENDS
		leechcraft-util-sll
		leechcraft-util-sys
	USES Widgets
	)
