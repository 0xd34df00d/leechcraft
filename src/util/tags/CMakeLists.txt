include_directories (${CMAKE_CURRENT_BINARY_DIR})
set (TAGS_SRCS
	categoryselector.cpp
	tagscompleter.cpp
	tagscompletionmodel.cpp
	tagslineedit.cpp
	tagsfiltermodel.cpp
	)
set (TAGS_FORMS
	categoryselector.ui
	)

if (USE_QT5)
	qt5_wrap_ui (TAGS_UIS_H ${TAGS_FORMS})
else ()
	QT4_WRAP_UI (TAGS_UIS_H ${TAGS_FORMS})
endif ()

foreach (SRC ${TAGS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/tags/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${TAGS_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/tags/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-tags SHARED
	${TAGS_SRCS}
	${TAGS_UIS_H}
	)
target_link_libraries (leechcraft-util-tags
	leechcraft-util-gui
	)
set_property (TARGET leechcraft-util-tags PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-tags DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-tags Widgets)
