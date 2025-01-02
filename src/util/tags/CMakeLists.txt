set (TAGS_SRCS
	categoryselector.cpp
	tagscompleter.cpp
	tagscompletionmodel.cpp
	tagslineedit.cpp
	tagsfiltermodel.cpp
	util.cpp
	)
set (TAGS_FORMS
	categoryselector.ui
	)

qt_wrap_ui (TAGS_UIS_H ${TAGS_FORMS})

add_util_library (util-tags
	SOURCES ${TAGS_SRCS} ${TAGS_UIS_H}
	DEPENDS leechcraft-util-gui
	USES Widgets
	)
