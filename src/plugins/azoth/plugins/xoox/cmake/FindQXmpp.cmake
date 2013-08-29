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
	HINTS
	${QXMPP_DIR}/include
	PATH
	ENV
)
find_library(QXMPP_LIBRARIES
	NAMES
	qxmpp0
	qxmpp
	HINTS
	${QXMPP_DIR}/lib
)

if(QXMPP_LOCAL)
	find_path(QXMPP_INCLUDE_DIR client/QXmppClient.h "${QXMPP_LOCAL}/src")
	if(QXMPP_INCLUDE_DIR)
		set(QXMPP_LIBRARIES ${QXMPP_LOCAL}/build/src/libqxmpp0.a)
	endif()
endif()

if(QXMPP_LIBRARIES AND QXMPP_INCLUDE_DIR)
	if(NOT QXMPP_LOCAL)
		set(QXMPP_INCLUDE_DIR
			${QXMPP_INCLUDE_DIR}/qxmpp
		)
	endif()
	set(QXMPP_FOUND 1)
endif()

if(QXMPP_FOUND)
	message(STATUS "Found QXmpp libraries at ${QXMPP_LIBRARIES}")
	message(STATUS "Found QXmpp headers at ${QXMPP_INCLUDE_DIR}")
else()
	if(QXMPP_FIND_REQUIRED)
		message(FATAL_ERROR "Could NOT find required QXmpp library, aborting")
	else()
		message(STATUS "Could NOT find QXmpp")
	endif()
endif()
