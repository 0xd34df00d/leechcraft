set (SLL_SRCS
	delayedexecutor.cpp
	onetimerunner.cpp
	queuemanager.cpp
	slotclosure.cpp
	)

foreach (SRC ${SLL_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/sll/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-sll SHARED
	${SLL_SRCS}
	)
target_link_libraries (leechcraft-util-sll
	${QT_LIBRARIES}
	)
set_property (TARGET leechcraft-util-sll PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-sll DESTINATION ${LIBDIR})
