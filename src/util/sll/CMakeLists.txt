set (SLL_SRCS
	delayedexecutor.cpp
	onetimerunner.cpp
	queuemanager.cpp
	regexp.cpp
	slotclosure.cpp
	)

option (WITH_PCRE "Use PCRE regular expression engine for instead of Qt's one (recommended since QRegExps are slow)." ON)
if (WITH_PCRE)
	set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
	find_package (PCRE REQUIRED)
	message (STATUS "Found PCRE includes: ${PCRE_INCLUDE_DIR}")
	message (STATUS "Found PCRE libraries: ${PCRE_LIBRARIES}")
	if (PCRE_INCLUDE_DIR AND PCRE_LIBRARIES)
		message (STATUS "Found PCRE, support for AdBlock+ separator-matching will be built")
	else ()
		message (FATAL_ERROR "PCRE support isn't available")
		set (PCRE_LIBRARIES)
	endif ()

	add_definitions (-DUSE_PCRE)
	include_directories (${PCRE_INCLUDE_DIR})
endif ()

foreach (SRC ${SLL_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/sll/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-sll SHARED
	${SLL_SRCS}
	)
target_link_libraries (leechcraft-util-sll
	${QT_LIBRARIES}
	${PCRE_LIBRARIES}
	)
set_property (TARGET leechcraft-util-sll PROPERTY SOVERSION 0.6.65)
install (TARGETS leechcraft-util-sll DESTINATION ${LIBDIR})
