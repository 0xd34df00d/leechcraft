# Find QXmpp library

#
# QXMPP_INCLUDE_DIR
# QXMPP_LIBRARIES
# QXMPP_FOUND

# Copyright (c) 2010-2012 Georg Rudoy <0xd34df00d@gmail.com>
# Copyright (c) 2012 Minh Ngo <nlminhtl@gmail.com>
# Win32 additions by Eugene Mamin <TheDZhon@gmail.com>
# Win32 fixes by Dimtriy Ryazantcev <DJm00n@mail.ru>

find_path(QXMPP_INCLUDE_DIR
	NAMES
	qxmpp/QXmppClient.h
	PATH
	ENV
)
find_library(QXMPP_LIBRARIES
	NAMES
	qxmpp0
	qxmpp
)

if(QXMPP_LOCAL)
	find_path(QXMPP_INCLUDE_DIR client/QXmppClient.h "${QXMPP_LOCAL}/src")
	if(QXMPP_INCLUDE_DIR)
		set(QXMPP_LIBRARIES ${QXMPP_LOCAL}/build/src/libqxmpp0.a)
	endif(QXMPP_INCLUDE_DIR)
endif(QXMPP_LOCAL)

if(QXMPP_LIBRARIES AND QXMPP_INCLUDE_DIR)
	if(NOT QXMPP_LOCAL)
		set(QXMPP_INCLUDE_DIR
			${QXMPP_INCLUDE_DIR}/qxmpp
		)
	endif(NOT QXMPP_LOCAL)
	set(QXMPP_FOUND 1)
endif(QXMPP_LIBRARIES AND QXMPP_INCLUDE_DIR)

if(QXMPP_FOUND)
	message(STATUS "Found QXmpp libraries at ${QXMPP_LIBRARIES}")
	message(STATUS "Found QXmpp headers at ${QXMPP_INCLUDE_DIR}")
else(QXMPP_FOUND)
	if(QXMPP_FIND_REQUIRED)
		message(FATAL_ERROR "Could NOT find required QXmpp library, aborting")
	else()
		message(STATUS "Could NOT find QXmpp")
	endif()
endif()
