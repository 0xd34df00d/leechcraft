add_util_library (util-tags
	SOURCES
		categoryselector.cpp
		tagscompleter.cpp
		tagscompletionmodel.cpp
		tagslineedit.cpp
		tagsfiltermodel.cpp
		util.cpp
	DEPENDS leechcraft-util-gui
	USES Widgets
	)
