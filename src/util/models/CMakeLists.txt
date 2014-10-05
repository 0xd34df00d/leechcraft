set (MODELS_SRCS
	flattenfiltermodel.cpp
	flattofoldersproxymodel.cpp
	mergemodel.cpp
	modeliterator.cpp
	modelitem.cpp
	)

foreach (SRC ${MODELS_SRCS})
	set (UTIL_SRCS "${UTIL_SRCS};util/models/${SRC}")
endforeach ()
set (UTIL_SRCS ${UTIL_SRCS} PARENT_SCOPE)

add_library (leechcraft-util-models${LC_LIBSUFFIX} SHARED
	${MODELS_SRCS}
	)
target_link_libraries (leechcraft-util-models${LC_LIBSUFFIX}
	)
set_property (TARGET leechcraft-util-models${LC_LIBSUFFIX} PROPERTY SOVERSION ${LC_SOVERSION})
install (TARGETS leechcraft-util-models${LC_LIBSUFFIX} DESTINATION ${LIBDIR})

FindQtLibs (leechcraft-util-models${LC_LIBSUFFIX} WebKitWidgets Widgets)
