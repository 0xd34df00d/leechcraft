include_directories (${CMAKE_CURRENT_BINARY_DIR})
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

foreach (SRC ${TAGS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/tags/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${TAGS_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/tags/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_util_library (util-tags
	SOURCES ${TAGS_SRCS} ${TAGS_UIS_H}
	DEPENDS leechcraft-util-gui
	USES Widgets
	)
