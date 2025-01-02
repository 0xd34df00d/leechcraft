set (TAGS_SRCS
	categoryselector.cpp
	tagscompleter.cpp
	tagscompletionmodel.cpp
	tagslineedit.cpp
	tagsfiltermodel.cpp
	util.cpp
	)

add_util_library (util-tags
	SOURCES ${TAGS_SRCS}
	DEPENDS leechcraft-util-gui
	USES Widgets
	)
