add_util_library (util-tags
	SRCS
		categoryselector.cpp
		tagscompleter.cpp
		tagscompletionmodel.cpp
		tagseditdelegate.cpp
		tagslineedit.cpp
		tagsfiltermodel.cpp
		util.cpp
	DEPENDS leechcraft-util-gui
	USES Widgets
	)
