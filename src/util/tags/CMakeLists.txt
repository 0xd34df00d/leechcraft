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

qt5_wrap_ui (TAGS_UIS_H ${TAGS_FORMS})

foreach (SRC ${TAGS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/tags/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

foreach (FORM ${TAGS_FORMS})
	set (UTIL_FORMS "${UTIL_FORMS};util/tags/${FORM}")
endforeach ()
set (UTIL_FORMS ${UTIL_FORMS} PARENT_SCOPE)

add_library (leechcraft-util-tags${LC_LIBSUFFIX} SHARED
	${TAGS_SRCS}
	${TAGS_UIS_H}
	)
target_link_libraries (leechcraft-util-tags${LC_LIBSUFFIX}
	leechcraft-util-gui${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-tags${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION}.1)
install (TARGETS leechcraft-util-tags${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-tags${LC_LIBSUFFIX} Widgets)
