set (TAGS_SRCS
	categoryselector.cpp
	tagscompleter.cpp
	tagscompletionmodel.cpp
	tagslineedit.cpp
	tagsfiltermodel.cpp
	)

foreach (SRC ${TAGS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/tags/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-tags SHARED
	${TAGS_SRCS}
	)
target_link_libraries (leechcraft-util-tags
	${QT_LIBRARIES}
	leechcraft-util-gui
	)
set_property (TARGET leechcraft-util-tags PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-tags DESTINATION ${LIBDIR})
